#ifndef TELEMETRY_STORAGE_H
#define TELEMETRY_STORAGE_H

#include <cstdint>
#include <memory>
#include <vector>

#include "logging/logger.h"
#include "packet/telemetry-packet.hpp"
#include "spdlog/logger.h"

namespace pqxx {
class connection;
}  // namespace pqxx

namespace telemetry {

#define TEL_STORAGE_STATE(X) \
    X(INITIAL)               \
    X(CONNECTED)             \
    X(ERROR)

enum class TelemetryStorageState {
#define GENERATE_ENUM_VALUE(v) v,
    TEL_STORAGE_STATE(GENERATE_ENUM_VALUE)
#undef GENERATE_ENUM_VALUE
};

constexpr const char *toString(TelemetryStorageState s) {
    switch (s) {
#define GENERATE_CASE(v)           \
    case TelemetryStorageState::v: \
        return #v;
        TEL_STORAGE_STATE(GENERATE_CASE)
#undef GENERATE_CASE
    }
    return "INITIAL";
}

class TelemetryStorage {
public:
    explicit TelemetryStorage(uint16_t batchSize);

    TelemetryStorageState connect();

    void add(const TelemetryPacket &packet);

    void flush();

    ~TelemetryStorage();
    TelemetryStorage(const TelemetryStorage &)            = delete;
    TelemetryStorage &operator=(const TelemetryStorage &) = delete;
    TelemetryStorage(TelemetryStorage &&other)            = delete;
    TelemetryStorage &operator=(TelemetryStorage &&other) = delete;

private:
    uint16_t                          batchSize_;
    TelemetryStorageState             state_{TelemetryStorageState::INITIAL};
    std::vector<TelemetryPacket>      batch_{};
    std::unique_ptr<pqxx::connection> conn_{nullptr};

    static std::shared_ptr<spdlog::logger> &log() {
        static auto instance = lib::Logger::get("TelemetryStorage");
        return instance;
    }
};
}  // namespace telemetry

#endif