#include "app/telemetry-ingestor/telemetry-ingestor.h"
#include "telemetry/packet/telemetry-parser.hpp"
#include "telemetry/packet/telemetry-helpers.hpp"
#include "libs/spsc-queue/spsc-queue.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>

namespace app
{

constexpr bool DEBUG_PACKETS = false;

TelemetryIngestor::TelemetryIngestor(std::string sockPath)
    : afUnixSock_{std::make_unique<lib::AfUnixUdpSocket>(std::move(sockPath))},
      spscQueue_{std::make_unique<lib::SPSCQueue<telemetry::TelemetryPacket>>()}
{
}

TelemetryIngestor::~TelemetryIngestor()
{
    stop();
}

void TelemetryIngestor::stop()
{
    running_ = false;
    if (afUnixSock_ && afUnixSock_->sockFd() > -1) {
        shutdown(afUnixSock_->sockFd(), SHUT_RDWR);
    }
}

void TelemetryIngestor::consume()
{
    telemetry::TelemetryPacket packet{};

    while (running_) {
        if (spscQueue_->pop(packet)) {
            spscQueueStats_.queue_read++;
            // TODO: Store telemetry or forward it
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    }
}

TelemetryIngestorStartResult TelemetryIngestor::run()
{
    auto bindResult = afUnixSock_->bind();
    if (bindResult != lib::AfUnixSocketState::BIND) {
        return {false, std::string{"Bind error"}};
    }

    int afUnixSockFd = afUnixSock_->sockFd();

    if (listen(afUnixSockFd, 1) < 0) {
        return {false, std::string{"Listen error"}};
    }

    std::cout << "Listening on af-unix with sock-fd: " << afUnixSockFd << std::endl;

    running_ = true;
    std::thread consumerThread([this]() { consume(); });

    while (running_) {
        int client_fd = accept(afUnixSockFd, nullptr, nullptr);
        if (client_fd < 0) {
            if (!running_) {
                break;
            }
            perror("accept error");
            continue;
        }

        std::cout << "Client connected!" << std::endl;

        telemetry::TelemetryParseStats sockStats;
        while (running_) {
            telemetry::TelemetryPacket packet{};
            if (!telemetry::readPacket(client_fd, packet, sockStats)) {
                break;
            }

            // Net to host
            telemetry::ntoh(packet);

            if constexpr (DEBUG_PACKETS) {
                // telemetry::printPacket(packet);
            }

            if (spscQueue_->push(packet)) {
                spscQueueStats_.queue_pushed++;
            } else {
                spscQueueStats_.queue_dropped++;
            }
        }

        std::cout << "Client " << client_fd << " stats: reads " << sockStats.reads
                  << " partials " << sockStats.partial_reads << std::endl;

        close(client_fd);
    }

    if (consumerThread.joinable()) {
        consumerThread.join();
    }
    std::cout.flush();

    return {true, "OK"};
}

} // namespace app
