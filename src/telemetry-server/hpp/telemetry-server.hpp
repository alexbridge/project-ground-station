#ifndef CLANG_TELEMETRY_SERVER
#define CLANG_TELEMETRY_SERVER

#include <atomic>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include "telemetry.hpp"
#include <cstddef>

extern std::atomic<bool> running;

int runServer();

void shutdownServer();

inline void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        running = false;
        shutdownServer();
    }
}

inline bool readPacket(int fd, TelemetryPacket &packet, ServerSockStats &stats)
{
    size_t total = 0;
    const size_t fullSize = sizeof(TelemetryPacket);

    std::byte *bytes = reinterpret_cast<std::byte *>(&packet);

    while (total < fullSize)
    {
        ssize_t n = read(fd, bytes + total, fullSize - total);
        if (n <= 0)
        {
            return false;
        }

        if (total == 0 && static_cast<size_t>(n) < fullSize)
        {
            stats.partial_reads++;
        }

        total += static_cast<size_t>(n);
    }

    stats.reads++;
    return true;
}

#endif