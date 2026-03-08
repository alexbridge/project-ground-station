#include "apps/af-unix-telemetry-server/af-unix-telemetry-server.h"
#include "hpp/telemetry-server.hpp"
#include "hpp/telemetry.hpp"
#include "socket/af-unix-socket.h"
#include "socket/udp-socket.h"
#include "spsc-queue/spsc-queue.hpp"
#include "storage/telemetry-storage.hpp"
#include <atomic>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <utility>

namespace app
{

constexpr bool DEBUG_PACKETS = false;

AfUnixTelemetryServer::AfUnixTelemetryServer(std::string sockPath)
{
    lib::AfUnixUdpSocket afUnixpSock_{std::move(sockPath)};
}

AfUnixTelemetryServerStartResult AfUnixTelemetryServer::run()
{
}

void AfUnixTelemetryServer::stop()
{
}

void consume(SPSCQueue<TelemetryPacket> &spscQueue,
             SPSCQueueStats &queueStats)
{
    TelemetryPacket packet{};
    TelemetryStorage telemetryStorage{};

    while (running) {
        // 1. Ensure connection is active
        if (!telemetryStorage.connected()) {
            if (!telemetryStorage.connect()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
        }

        if (spscQueue.pop(packet)) {
            queueStats.queue_read++;
            telemetryStorage.add(packet);
        } else {
            // If we've been idle, flush partial batch
            telemetryStorage.flush();
            // Release cpu cyles
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    }

    // Flush leftover
    telemetryStorage.flush();
};

AfUnixTelemetryServerStartResult AfUnixTelemetryServer::run()
{
    auto bind = afUnixSock_->bind();
    if (bind != lib::AfUnixSocketState::BIND) {
        return {false, std::string{"Bind error"}};
    }

    int afUnixSock = afUnixSock_->sockFd();

    if (listen(afUnixSock, 1) < 0) {
        return {false, std::string{"Listen error"}};
    }

    std::cout << "Listening on af-unix with sock-fd: " << afUnixSock << std::endl;

    std::thread consumer(
        [&spscQueue, &queueStats]() { consume(spscQueue, queueStats); });

    while (running) {
        int client_fd = accept(sock_fd, nullptr, nullptr);
        if (client_fd < 0) {
            if (!running) {
                break;
            }
            perror("accept error");
            break;
        }

        std::cout << "Client connected!" << std::endl;

        while (running) {
            TelemetryPacket packet{};
            if (!readPacket(client_fd, packet, sockStats)) {
                break;
            }

            // Net to host
            ntoh(packet);

            if constexpr (DEBUG_PACKETS) {
                printAscii(packet);
                printPacket(packet);
            }

            if (spscQueue.push(packet)) {
                queueStats.queue_pushed++;
            } else {
                queueStats.queue_dropped++;
            }
        }

        std::cout << "Client " << client_fd << " stats: reads " << sockStats.reads
                  << " stats: partials " << sockStats.partial_reads << std::endl;

        close(client_fd);
    }

    consumer.join();
    std::cout.flush();

    return 0;
}

void shutdownServer()
{
    if (socketFd > -1) {
        shutdown(socketFd, SHUT_RDWR);
    }
}