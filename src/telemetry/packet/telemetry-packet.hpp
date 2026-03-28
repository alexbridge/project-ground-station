#ifndef TELEMETRY_PACKET_H
#define TELEMETRY_PACKET_H

#include <cstdint>
#include <cstring>

namespace telemetry
{

#pragma pack(push, 1)
struct TelemetryPacket {
    uint16_t appId;     // 2 bytes
    uint32_t timestamp; // 4 bytes
    float batteryV;     // 4 bytes

    float batteryVoltage() const
    {
        float v{};
        std::memcpy(&v, &batteryV, sizeof(v));
        return v;
    }
};
#pragma pack(pop)

} // namespace telemetry

#endif