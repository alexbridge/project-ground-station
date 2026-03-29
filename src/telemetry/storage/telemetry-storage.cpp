#include "storage/telemetry-storage.hpp"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <string>

#include "packet/telemetry-helpers.hpp"
#include "utils/env-utils.h"

namespace telemetry {

const std::string     TM_STORAGE_HOST  = lib::getEnv("TELEMETRY_DB_HOST", "localhost");
constexpr const char *TM_STORAGE_PORT  = "5432";
constexpr const char *TM_STORAGE_DB    = "GROUND_STATION";
constexpr const char *TM_STORAGE_TABLE = "TELEMETRY";
constexpr const char *TM_STORAGE_USER  = "ground";
constexpr const char *TM_STORAGE_PASS  = "ground";

TelemetryStorageState telemetry::TelemetryStorage::connect() {
    if (state_ == TelemetryStorageState::CONNECTED) {
        return state_;
    }

    std::string connStr = fmt::format(
        "host={} port={} dbname={} user={} password={}",
        TM_STORAGE_HOST,
        TM_STORAGE_PORT,
        TM_STORAGE_DB,
        TM_STORAGE_USER,
        TM_STORAGE_PASS
    );

    try {
        conn_  = std::make_unique<pqxx::connection>(connStr);
        state_ = TelemetryStorageState::CONNECTED;
        return state_;
    } catch (const std::exception &e) {
        log()->error("DB Connection Error: {}", e.what());
        state_ = TelemetryStorageState::ERROR;
        return state_;
    }
}

void TelemetryStorage::flush() {
    if (batch_.empty()) {
        return;
    }

    if (!conn_) {
        connect();
        if (connect() != TelemetryStorageState::CONNECTED) {
            // TODO Clear batch buffer for consequitive error
            return;
        }
    }

    try {
        pqxx::work tx(*conn_);

        auto writer = pqxx::stream_to::table(tx, {TM_STORAGE_TABLE}, {"app_id", "time", "voltage"});

        for (const auto &packet : batch_) {
            // Convert uint32_t timestamp (unix epoch) to a format Postgres

            std::time_t t = static_cast<std::time_t>(packet.alignedTimestamp());

            struct tm tm_buf;
            gmtime_r(&t, &tm_buf);

            char time_str[20];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

            writer << std::make_tuple(packet.appId, time_str, packet.alignedVoltage());
        }

        writer.complete();
        tx.commit();

        batch_.clear();
    } catch (const std::exception &e) {
        std::cerr << "DB Flush Error: " << e.what() << ". Resetting connection..." << std::endl;

        state_ = TelemetryStorageState::ERROR;
        conn_.reset();
    }
}

void TelemetryStorage::add(const TelemetryPacket &packet) {
    batch_.push_back(packet);

    if (batch_.size() >= batchSize_) {
        flush();
    }
}

TelemetryStorage::TelemetryStorage(uint16_t batchSize) : batchSize_(batchSize) { batch_.reserve(batchSize); }

TelemetryStorage::~TelemetryStorage() {
    // any remaining
    flush();
}

}  // namespace telemetry
