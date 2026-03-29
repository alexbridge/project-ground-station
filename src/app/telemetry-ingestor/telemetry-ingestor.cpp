#include "telemetry-ingestor/telemetry-ingestor.h"

#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <thread>

#include "packet/telemetry-helpers.hpp"
#include "packet/telemetry-parser.hpp"
#include "storage/telemetry-storage.hpp"

namespace app {

constexpr bool DEBUG_PACKETS = false;

TelemetryIngestor::TelemetryIngestor(std::string sockPath)
    : afUnixSock_{std::make_unique<lib::AfUnixUdpSocket>(std::move(sockPath))},
      spscQueue_{std::make_unique<lib::SPSCQueue<telemetry::TelemetryPacket>>()},
      storage_{std::make_unique<telemetry::TelemetryStorage>(1024)} {}

TelemetryIngestor::~TelemetryIngestor() { stop(); }

void TelemetryIngestor::stop() {
    running_ = false;
    if (afUnixSock_ && afUnixSock_->sockFd() > -1) {
        shutdown(afUnixSock_->sockFd(), SHUT_RDWR);
    }
}

void TelemetryIngestor::consume() {
    auto storageState = telemetry::TelemetryStorageState::INITIAL;

    // @TODO: consume not ready, what to do?
    for (size_t i = 0; i < 100; i++) {
        storageState = storage_->connect();
        if (storageState == telemetry::TelemetryStorageState::CONNECTED) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    log()->info("Ingestor consumer ready, storage {}", toString(storageState));

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

TelemetryIngestorStartResult TelemetryIngestor::run() {
    auto bindResult = afUnixSock_->bind();
    if (bindResult != lib::AfUnixSocketState::BIND) {
        return {false, std::string{"Bind error"}};
    }

    int afUnixSockFd = afUnixSock_->sockFd();

    log()->info("AF-unix sock-fd: {}", afUnixSockFd);

    running_ = true;
    std::thread                    consumerThread([this]() { consume(); });
    telemetry::TelemetryParseStats sockStats;

    while (running_) {
        telemetry::TelemetryPacket packet{};
        if (!telemetry::readPacket(afUnixSockFd, packet, sockStats)) {
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

    if (consumerThread.joinable()) {
        consumerThread.join();
    }

    return {true, "OK"};
}

}  // namespace app
