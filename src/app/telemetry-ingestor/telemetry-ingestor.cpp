#include "telemetry-ingestor/telemetry-ingestor.h"

#include <sys/socket.h>
#include <unistd.h>

#include "packet/telemetry-helpers.hpp"
#include "packet/telemetry-parser.hpp"

namespace app
{

constexpr bool DEBUG_PACKETS = false;

TelemetryIngestor::TelemetryIngestor(std::string sockPath)
    : afUnixSock_{std::make_unique<lib::AfUnixUdpSocket>(std::move(sockPath))},
      spscQueue_{std::make_unique<lib::SPSCQueue<telemetry::TelemetryPacket>>()},
      storage_{std::make_unique<telemetry::TelemetryStorage>(1024)}
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
    // TODO Is it always same reference for every packets?
    telemetry::TelemetryPacket packet{};

    while (running_) {
        if (spscQueue_->pop(packet)) {
            spscQueueStats_.queue_read++;
            storage_->add(packet);
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

    log()->info("Listening on af-unix: sock-fd {}", afUnixSockFd);

    running_ = true;
    std::thread consumerThread([this]() { consume(); });

    while (running_) {
        int clientFd = accept(afUnixSockFd, nullptr, nullptr);
        if (clientFd < 0) {
            if (!running_) {
                break;
            }
            perror("accept error");
            continue;
        }

        log()->info("Client connected: {}. Run thread", clientFd);

        telemetry::TelemetryParseStats sockStats;
        while (running_) {
            telemetry::TelemetryPacket packet{};
            if (!telemetry::readPacket(clientFd, packet, sockStats)) {
                break;
            }

            // Net to host
            telemetry::ntoh(packet);

            if constexpr (DEBUG_PACKETS) {
                telemetry::printPacket(packet);
            }

            if (spscQueue_->push(packet)) {
                spscQueueStats_.queue_pushed++;
            } else {
                spscQueueStats_.queue_dropped++;
            }
        }

        log()->info(
            "Client {} stats: reads {}, partials {} ",
            sockStats.reads,
            sockStats.partial_reads);

        close(clientFd);
    }

    if (consumerThread.joinable()) {
        consumerThread.join();
    }

    return {true, "OK"};
}

} // namespace app
