#ifndef CLANG_TELEMETRY_SERVER
#define CLANG_TELEMETRY_SERVER

#include "telemetry-packet.hpp"
#include <cstddef>
#include <unistd.h>

namespace telemetry
{

struct TelemetryParseStats {
    size_t reads{0};
    size_t partial_reads{0};
};

inline bool readPacket(int fd, TelemetryPacket &packet, TelemetryParseStats &stats)
{
    size_t total = 0;
    const size_t fullSize = sizeof(TelemetryPacket);

    std::byte *bytes = reinterpret_cast<std::byte *>(&packet);

    while (total < fullSize) {
        ssize_t n = read(fd, bytes + total, fullSize - total);
        if (n <= 0) {
            return false;
        }

        if (total == 0 && static_cast<size_t>(n) < fullSize) {
            stats.partial_reads++;
        }

        total += static_cast<size_t>(n);
    }

    stats.reads++;
    return true;
}

} // namespace telemetry

#endif