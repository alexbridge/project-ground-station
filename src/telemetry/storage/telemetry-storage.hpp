#ifndef TELEMETRY_STORAGE_H
#define TELEMETRY_STORAGE_H

#include <cstdint>
#include <fmt/core.h>
#include <memory>
#include <vector>

#include "packet/telemetry-packet.hpp"

namespace pqxx
{
class connection;
} // namespace pqxx

namespace telemetry
{
enum class TelemetryStorageState { INITIAL,
                                   CONNECTED,
                                   ERROR };

class TelemetryStorage
{
public:
    explicit TelemetryStorage(uint16_t batchSize);

    TelemetryStorageState connect();

    void add(const TelemetryPacket &packet);

    void flush();

    ~TelemetryStorage();
    TelemetryStorage(const TelemetryStorage &) = delete;
    TelemetryStorage &operator=(const TelemetryStorage &) = delete;
    TelemetryStorage(TelemetryStorage &&other) = delete;
    TelemetryStorage &operator=(TelemetryStorage &&other) = delete;

private:
    uint16_t batchSize_;
    TelemetryStorageState state_{TelemetryStorageState::INITIAL};
    std::vector<TelemetryPacket> batch_{};
    std::unique_ptr<pqxx::connection> conn_{nullptr};
};
} // namespace telemetry

#endif