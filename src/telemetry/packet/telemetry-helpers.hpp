#ifndef TELEMETRY_HELPERS_H
#define TELEMETRY_HELPERS_H

#include <cstring>
#include <fmt/core.h>
#include <netinet/in.h>

#include "telemetry-packet.hpp"

namespace telemetry
{

inline void printPacket(const TelemetryPacket &packet)
{
    fmt::print("App {}, at {}, battery {:.2f}\n", packet.appId, packet.timestamp, packet.batteryVoltage());
}

inline void printAscii(const TelemetryPacket &packet)
{
    const uint8_t *bytes = reinterpret_cast<const std::uint8_t *>(&packet);

    fmt::print("Telemetry packet ASCII[\n");
    for (size_t i = 0; i < sizeof(TelemetryPacket); i++) {
        fmt::print("{}{}", static_cast<int>(bytes[i]), i == sizeof(TelemetryPacket) - 1 ? "" : ", ");
    }
    fmt::print("]\n");
}

inline void hton(TelemetryPacket &packet)
{
    packet.appId = htons(packet.appId);
    packet.timestamp = htonl(packet.timestamp);

    uint32_t netBatteryV;
    memcpy(&netBatteryV, &packet.batteryV, sizeof(netBatteryV));
    netBatteryV = htonl(netBatteryV);

    memcpy(&packet.batteryV, &netBatteryV, sizeof(netBatteryV));
}

inline void ntoh(TelemetryPacket &packet)
{
    packet.appId = ntohs(packet.appId);
    packet.timestamp = ntohl(packet.timestamp);

    uint32_t hostBatteryV;
    memcpy(&hostBatteryV, &packet.batteryV, sizeof(hostBatteryV));
    hostBatteryV = ntohl(hostBatteryV);

    memcpy(&packet.batteryV, &hostBatteryV, sizeof(hostBatteryV));
}

} // namespace telemetry
#endif