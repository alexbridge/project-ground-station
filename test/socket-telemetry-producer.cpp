#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>
#include <bits/this_thread_sleep.h>
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

    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        perror("socket start error");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, TELEMETRY_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(client_fd);
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    for (int i = 0; i < numEvents; ++i)
    {
        std::cout << "Sending: " << i << " ... ";

        TelemetryPacket packet = generateTelemetry(rangeStart, rangeEnd);

        printPacket(packet);

        ssize_t sent = send(
            client_fd,
            reinterpret_cast<const char *>(&packet),
            sizeof(packet),
            0);
        if (sent <= 0)
        {
            perror("error");
            break;
        }

        std::cout << " sent" << std::endl;

        if (i % 100 == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    close(client_fd);
    return 0;
}