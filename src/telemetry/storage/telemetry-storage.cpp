#include "storage/telemetry-storage.hpp"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <string>

#include "packet/telemetry-helpers.hpp"

namespace telemetry {

constexpr const char *TM_STORAGE_HOST  = "localhost";
constexpr const char *TM_STORAGE_PORT  = "5432";
constexpr const char *TM_STORAGE_DB    = "GROUND_STATION";
constexpr const char *TM_STORAGE_TABLE = "TELEMETRY";
constexpr const char *TM_STORAGE_USER  = "ground";
constexpr const char *TM_STORAGE_PASS  = "ground";

TelemetryStorageState telemetry::TelemetryStorage::connect() {
    std::string connStr = std::string("host=") + TM_STORAGE_HOST + " port=" + TM_STORAGE_PORT +
                          " dbname=" + TM_STORAGE_DB + " user=" + TM_STORAGE_USER +
                          " password=" + TM_STORAGE_PASS;

    try {
        conn_  = std::make_unique<pqxx::connection>(connStr);
        state_ = TelemetryStorageState::CONNECTED;
        return state_;
    } catch (const std::exception &e) {
        std::cerr << "DB Connection Error: " << e.what() << ". Retrying in 1s..." << std::endl;
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

        // Using stream_to to bulk insert into TELEMETRY table (v6.4.5 compatible)
        // Columns: time, app_id, voltage
        auto writer = pqxx::stream_to::table(tx, {TM_STORAGE_TABLE}, {"time", "app_id", "voltage"});

        for (const auto &packet : batch_) {
            // Convert uint32_t timestamp (unix epoch) to a format Postgres
            // understands

            std::time_t t = static_cast<std::time_t>(packet.timestamp);

            struct tm tm_buf;
            gmtime_r(&t, &tm_buf);

            char time_str[20];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

            writer << std::make_tuple(time_str, packet.appId, packet.alignedVoltage());
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

TelemetryStorage::TelemetryStorage(uint16_t batchSize) : batchSize_(batchSize) {
    batch_.reserve(batchSize);
}

TelemetryStorage::~TelemetryStorage() {
    // any remaining
    flush();
}

}  // namespace telemetry
