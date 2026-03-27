#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <random>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "logging/logger.h"
#include "packet/telemetry-helpers.hpp"
#include "packet/telemetry-packet.hpp"
#include "socket/udp-socket.h"

telemetry::TelemetryPacket generateTelemetry(uint16_t rangeStart, uint16_t rangeEnd)
{
    static std::default_random_engine eng{std::random_device{}()};

    // 16-bit appId
    static std::uniform_int_distribution<uint16_t> appIdDist(rangeStart, rangeEnd);

    // Current timestamp (seconds since epoch)
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    uint32_t currentTimestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(epoch).count());

    // float battery voltage
    static std::uniform_real_distribution<float> voltDist(3.3f, 4.2f);

    return telemetry::TelemetryPacket{
        appIdDist(eng),
        currentTimestamp,
        voltDist(eng)};
}

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <num_events> <app_id_start> <app_id_end>\n";
        return 1;
    }

    int numEvents = 0;
    uint16_t rangeStart = 0;
    uint16_t rangeEnd = 0;

    try {
        numEvents = std::stoi(argv[1]);
        rangeStart = static_cast<uint16_t>(std::max(std::stoi(argv[2]), 0));
        rangeEnd = static_cast<uint16_t>(std::max(std::stoi(argv[3]), 0));
    } catch (const std::exception &e) {
        std::cerr << "Invalid arguments.\n";
        return 1;
    }

    if (numEvents < 1) {
        std::cerr << "Number of events must be positive\n";
        return 1;
    }

    auto mainLogger = lib::Logger::get("UdpProducer");

    lib::UdpSocket udpSock{5005};

    auto res = udpSock.connect();
    if (res != lib::UdpSocketState::CONNECT) {
        mainLogger->error("UDP connect error");
        return 1;
    }

    int sockFd = udpSock.sockFd();

    mainLogger->info("Target: 127.0.0.1:5005 (UDP)");

    for (int i = 0; i < numEvents; ++i) {
        telemetry::TelemetryPacket packet = generateTelemetry(rangeStart, rangeEnd);

        telemetry::printPacket(packet);

        // Host to network
        hton(packet);

        ssize_t sent = send(
            sockFd,
            reinterpret_cast<const char *>(&packet),
            sizeof(packet),
            0,
            (const struct sockaddr *)&servaddr,
            sizeof(servaddr));

        if (sent <= 0) {
            perror("sendto error");
            break;
        }

        mainLogger->info(".");

        if (i % 1024 == 0) {
            mainLogger->info("1024\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    mainLogger->info("sent: {}", numEvents);

    return 0;
}