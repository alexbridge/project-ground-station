#ifndef TELEMETRY_PACKET_HELPERS_H
#define TELEMETRY_PACKET_HELPERS_H

#include "telemetry-packet.hpp"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>

namespace telemetry
{

constexpr static const char *TELEMETRY_SOCK_PATH = "/tmp/telemetry.sock";

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
    for (size_t i = 0; i < sizeof(TelemetryPacket); i++) {
        std::cout << static_cast<int>(bytes[i]) << (i == sizeof(TelemetryPacket) - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
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