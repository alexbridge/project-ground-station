#include "telemetry-storage.hpp"
#include <ctime>
#include <pqxx/pqxx>

constexpr static const char *TM_STORAGE_HOST = "localhost";
constexpr static const char *TM_STORAGE_PORT = "5432";
constexpr static const char *TM_STORAGE_DB = "GROUND_STATION";
constexpr static const char *TM_STORAGE_TABLE = "TELEMETRY";
constexpr static const char *TM_STORAGE_USER = "ground";
constexpr static const char *TM_STORAGE_PASS = "ground";

bool TelemetryStorage::connected() const {
  return conn != nullptr && conn->is_open();
}

bool TelemetryStorage::connect() {
  char conn_str[256];
  snprintf(conn_str, sizeof(conn_str),
           "host=%s port=%s dbname=%s user=%s password=%s", TM_STORAGE_HOST,
           TM_STORAGE_PORT, TM_STORAGE_DB, TM_STORAGE_USER, TM_STORAGE_PASS);

  try {
    conn = std::make_unique<pqxx::connection>(conn_str);
    std::cout << "Connected to " << TM_STORAGE_HOST << "/" << TM_STORAGE_DB
              << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "DB Connection Error: " << e.what() << ". Retrying in 1s..."
              << std::endl;
    return false;
  }
}

void TelemetryStorage::flush() {
  if (batch.empty() || !conn) {
    return;
  }

  try {
    pqxx::work tx(*conn);

    // Using stream_to to bulk insert into TELEMETRY table (v6.4.5 compatible)
    // Columns: time, app_id, voltage
    auto writer = pqxx::stream_to::table(tx, {TM_STORAGE_TABLE},
                                         {"time", "app_id", "voltage"});

    for (const auto &packet : batch) {
      // Convert uint32_t timestamp (unix epoch) to a format Postgres
      // understands

      std::time_t t = static_cast<std::time_t>(packet.timestamp);

      struct tm tm_buf;
      gmtime_r(&t, &tm_buf);

      char time_str[20];
      std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

      float voltage;
      std::memcpy(&voltage, &packet.batteryV, sizeof(voltage));
      writer << std::make_tuple(time_str, packet.appId, voltage);
    }

    writer.complete();
    tx.commit();

    batch.clear();
  } catch (const std::exception &e) {
    std::cerr << "DB Flush Error: " << e.what() << ". Resetting connection..."
              << std::endl;
    conn.reset();
  }
}

void TelemetryStorage::add(const TelemetryPacket &packet) {
  batch.push_back(packet);

  if (batch.size() >= batchSize) {
    flush();
  }
}

TelemetryStorage::TelemetryStorage() { batch.reserve(batchSize); }

TelemetryStorage::~TelemetryStorage() = default;
