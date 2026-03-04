#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <random>
#include <thread>
#include "../src/telemetry-server/hpp/telemetry.hpp"

#include <chrono>

TelemetryPacket generateTelemetry(uint16_t rangeStart, uint16_t rangeEnd)
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

    return TelemetryPacket{
        appIdDist(eng),
        currentTimestamp,
        voltDist(eng)};
}

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <num_events> <app_id_start> <app_id_end>\n";
        return 1;
    }

    int numEvents = 0;
    uint16_t rangeStart = 0;
    uint16_t rangeEnd = 0;

    try
    {
        numEvents = std::stoi(argv[1]);
        rangeStart = static_cast<uint16_t>(std::max(std::stoi(argv[2]), 0));
        rangeEnd = static_cast<uint16_t>(std::max(std::stoi(argv[3]), 0));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Invalid arguments.\n";
        return 1;
    }

    if (numEvents < 1)
    {
        std::cerr << "Number of events must be positive\n";
        return 1;
    }

    int client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0)
    {
        perror("socket create error");
        return 1;
    }

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5005);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::cout << "Target: 127.0.0.1:5005 (UDP)" << std::endl;

    for (int i = 0; i < numEvents; ++i)
    {
        TelemetryPacket packet = generateTelemetry(rangeStart, rangeEnd);

        printPacket(packet);

        // Host to network
        hton(packet);

        ssize_t sent = sendto(
            client_fd,
            reinterpret_cast<const char *>(&packet),
            sizeof(packet),
            0,
            (const struct sockaddr *)&servaddr,
            sizeof(servaddr));
        if (sent <= 0)
        {
            perror("sendto error");
            break;
        }

        std::cout << ".";

        if (i % 1024 == 0)
        {
            std::cout << " 1024\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::cout << " sent: " << numEvents << std::endl;

    close(client_fd);
    return 0;
}