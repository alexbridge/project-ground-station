#ifndef CLANG_TELEMETRY
#define CLANG_TELEMETRY

#include <cstdint>
#include <atomic>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <endian.h>
#include <netinet/in.h>
#include <arpa/inet.h>

constexpr static const char *TELEMETRY_SOCK_PATH = "/tmp/telemetry.sock";

#pragma pack(push, 1)
struct TelemetryPacket
{
    uint16_t appId;     // 2 bytes
    uint32_t timestamp; // 4 bytes
    float batteryV;     // 4 bytes
};
#pragma pack(pop)

struct ServerSockStats
{
    size_t reads{0};
    size_t partial_reads{0};
};

inline void printPacket(const TelemetryPacket &packet)
{
    std::cout
        << "App: " << packet.appId
        << " at " << packet.timestamp
        << " battery: " << std::fixed << std::setprecision(2) << packet.batteryV << "V" << std::endl;
}

inline void printAscii(const TelemetryPacket &packet)
{
    const uint8_t *bytes = reinterpret_cast<const std::uint8_t *>(&packet);

    std::cout << "Telemetry packet ASCII[";
    for (size_t i = 0; i < sizeof(TelemetryPacket); i++)
    {
        std::cout << static_cast<int>(bytes[i]) << (i == sizeof(TelemetryPacket) - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

inline void hton(TelemetryPacket &packet)
{
    packet.appId = htons(packet.appId);
    packet.timestamp = htonl(packet.timestamp);

    uint32_t net;
    memcpy(&net, &packet.batteryV, sizeof(net));
    net = htonl(net);

    memcpy(&packet.batteryV, &net, sizeof(net));
}

inline void ntoh(TelemetryPacket &packet)
{
    packet.appId = ntohs(packet.appId);
    packet.timestamp = ntohl(packet.timestamp);

    uint32_t net;
    memcpy(&net, &packet.batteryV, sizeof(net));
    net = ntohl(net);

    memcpy(&packet.batteryV, &net, sizeof(net));
}

#endif