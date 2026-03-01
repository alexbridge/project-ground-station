#include <cstring>
#include <string>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <csignal>
#include <atomic>
#include <thread>
#include <vector>
#include <pqxx/pqxx>
#include "hpp/telemetry.hpp"
#include "hpp/telemetry-server.hpp"
#include "spsc-queue/spsc-queue.hpp"

std::atomic<int> socketFd{-1};

void flush_to_db(pqxx::connection &conn, const std::vector<TelemetryPacket> &batch)
{
    pqxx::work tx(conn);

    // Using stream_to to bulk insert into TELEMETRY table (v6.4.5 compatible)
    // Columns: time, app_id, voltage
    pqxx::stream_to writer(tx, "TELEMETRY", std::vector<std::string>{"time", "app_id", "voltage"});

    for (const auto &packet : batch)
    {
        // Convert uint32_t timestamp (unix epoch) to a format Postgres understands
        std::time_t t = static_cast<std::time_t>(packet.timestamp);
        char time_str[20];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::gmtime(&t));

        writer << std::make_tuple(std::string(time_str), packet.appId, packet.batteryV);
    }

    writer.complete();
    tx.commit();
}

void consume(SPSCQueue<TelemetryPacket> &spscQueue, SPSCQueueStats &queueStats)
{
    TelemetryPacket packet{};
    const size_t BATCH_SIZE = 1000;
    std::vector<TelemetryPacket> dbBatch;
    dbBatch.reserve(BATCH_SIZE);

    const std::string conn_str = "host=localhost port=5432 dbname=TELEMETRY user=ground password=ground";
    std::unique_ptr<pqxx::connection> conn;

    while (running)
    {
        // 1. Ensure connection is active
        if (!conn)
        {
            try
            {
                conn = std::make_unique<pqxx::connection>(conn_str);
                std::cout << "[Consumer] Connected to DB." << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "[Consumer] DB Connection Error: " << e.what() << ". Retrying in 1s..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
        }

        if (spscQueue.pop(packet))
        {
            queueStats.queue_read++;
            dbBatch.push_back(packet);

            if (dbBatch.size() >= BATCH_SIZE)
            {
                try
                {
                    flush_to_db(*conn, dbBatch);
                    dbBatch.clear();
                }
                catch (const std::exception &e)
                {
                    std::cerr << "[Consumer] DB Flush Error: " << e.what() << ". Resetting connection..." << std::endl;
                    conn.reset(); // Force reconnection on next loop
                }
            }
        }
        else
        {
            // If we've been idle, flush partial batch
            if (!dbBatch.empty() && conn)
            {
                try
                {
                    flush_to_db(*conn, dbBatch);
                    dbBatch.clear();
                }
                catch (const std::exception &e)
                {
                    std::cerr << "[Consumer] DB Idle Flush Error: " << e.what() << std::endl;
                    conn.reset();
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    }

    if (!dbBatch.empty() && conn)
    {
        try
        {
            flush_to_db(*conn, dbBatch);
        }
        catch (...)
        {
        } // Final effort on shutdown
    }
};

int runServer()
{
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("Error creating socket.");
        return 1;
    }

    socketFd = sock_fd;

    unlink(TELEMETRY_SOCK_PATH);

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, TELEMETRY_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sock_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind error");
        close(sock_fd);
        return 1;
    }

    if (listen(sock_fd, 1) < 0)
    {
        perror("listen error");
        close(sock_fd);
        return 1;
    }

    std::cout << "Listening on " << TELEMETRY_SOCK_PATH << std::endl;

    std::vector<std::thread> clients;

    SPSCQueue<TelemetryPacket> spscQueue{};
    SPSCQueueStats queueStats{};
    ServerSockStats sockStats{};

    std::thread consumer([&spscQueue, &queueStats]()
                         { consume(spscQueue, queueStats); });

    while (running)
    {
        int client_fd = accept(sock_fd, nullptr, nullptr);
        if (client_fd < 0)
        {
            if (!running)
            {
                break;
            }
            perror("accept error");
            break;
        }

        std::cout << "Client connected!" << std::endl;

        while (running)
        {
            TelemetryPacket packet{};
            if (!readPacket(client_fd, packet, sockStats))
            {
                break;
            }

            printAscii(packet);
            printPacket(packet);

            if (spscQueue.push(packet))
            {
                queueStats.queue_pushed++;
            }
            else
            {
                queueStats.queue_dropped++;
            }
        }

        std::cout << "Client "
                  << client_fd
                  << " stats: reads "
                  << sockStats.reads
                  << " stats: partials "
                  << sockStats.partial_reads
                  << std::endl;

        close(client_fd);
    }

    consumer.join();
    std::cout.flush();

    close(sock_fd);
    unlink(TELEMETRY_SOCK_PATH);

    return 0;
}

void shutdownServer()
{
    if (socketFd > -1)
    {
        shutdown(socketFd, SHUT_RDWR);
    }
}