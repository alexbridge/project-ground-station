#include <bits/this_thread_sleep.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <random>

#include "app-commons.h"
#include "packet/telemetry-helpers.hpp"
#include "packet/telemetry-packet.hpp"
#include "socket/af-unix-socket.h"

telemetry::TelemetryPacket generateTelemetry(uint16_t rangeStart, uint16_t rangeEnd) {
    static std::default_random_engine randomEngine{std::random_device{}()};

    // 16-bit appId
    static std::uniform_int_distribution<uint16_t> appIdDist(rangeStart, rangeEnd);

    // Current timestamp (seconds since epoch)
    auto     now   = std::chrono::system_clock::now();
    auto     epoch = now.time_since_epoch();
    uint32_t currentTimestamp =
        static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(epoch).count());

    // float battery voltage
    static std::uniform_real_distribution<float> voltDist(3.3f, 4.2f);

    return telemetry::TelemetryPacket{
        appIdDist(randomEngine),
        currentTimestamp,
        voltDist(randomEngine)};
}

int main(int argc, char const *argv[]) {
    auto mainLogger = lib::Logger::get("AfUnixDgramProducer");
    if (argc != 4) {
        mainLogger->warn("Usage: {} <num_events> <app_id_start> <app_id_end>", argv[0]);
        return 1;
    }

    int      numEvents  = 0;
    uint16_t rangeStart = 0;
    uint16_t rangeEnd   = 0;

    try {
        numEvents  = std::stoi(argv[1]);
        rangeStart = static_cast<uint16_t>(std::max(std::stoi(argv[2]), 0));
        rangeEnd   = static_cast<uint16_t>(std::max(std::stoi(argv[3]), 0));
    } catch (const std::exception &e) {
        mainLogger->error("Invalid arguments: {}", fmt::join(argv + 1, argv + argc, ", "));
        return 1;
    }

    if (numEvents < 1) {
        mainLogger->error("Number of events must be positive, but was {}", numEvents);
        return 1;
    }

    lib::AfUnixUdpSocket afUnixUdpSock{app::TELEMETRY_SOCK_PATH};

    auto res = afUnixUdpSock.connect();
    if (res != lib::AfUnixSocketState::CONNECT) {
        mainLogger->error("Connect to af-uxix sock not possible");
        return 1;
    }

    int udpSockFd = afUnixUdpSock.sockFd();

    mainLogger->info(
        "AF-sock connected: {}, actual buffer size {}",
        udpSockFd,
        afUnixUdpSock.actualBufSize().value()
    );

    mainLogger->info("Sending {} events", numEvents);

    for (int i = 0; i < numEvents; ++i) {
        mainLogger->info("Sending {}", i);

        telemetry::TelemetryPacket packet = generateTelemetry(rangeStart, rangeEnd);

        telemetry::printPacket(packet);

        // Host to network
        hton(packet);

        ssize_t sent = send(udpSockFd, reinterpret_cast<const char *>(&packet), sizeof(packet), 0);
        if (sent <= 0) {
            perror("error");
            break;
        }

        mainLogger->info("sent");

        if (i % 100 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return 0;
}