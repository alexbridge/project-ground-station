#ifndef GROUND_STORAGE_TELEMETRY_HPP
#define GROUND_STORAGE_TELEMETRY_HPP

#include "../hpp/telemetry.hpp"
#include <memory>
#include <vector>

namespace pqxx {
class connection;
}

class TelemetryStorage {
private:
  static constexpr size_t batchSize = 1024;
  std::vector<TelemetryPacket> batch{};
  std::unique_ptr<pqxx::connection> conn;

public:
  TelemetryStorage();

  bool connected() const;

  bool connect();

  void add(const TelemetryPacket &packet);

  void flush();

  ~TelemetryStorage();
  TelemetryStorage(const TelemetryStorage &) = delete;
  TelemetryStorage &operator=(const TelemetryStorage &) = delete;
};

#endif