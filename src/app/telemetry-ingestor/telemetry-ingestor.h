#ifndef APP_TELEMETRY_INGESTOR_H
#define APP_TELEMETRY_INGESTOR_H

#include <memory>
#include <string>

#include "packet/telemetry-packet.hpp"
#include "socket/af-unix-socket.h"
#include "spsc-queue/spsc-queue.hpp"

namespace app
{

struct TelemetryIngestorStartResult {
    bool ok;
    std::string msg;
};

class TelemetryIngestor
{
public:
    explicit TelemetryIngestor(std::string sockPath);

    ~TelemetryIngestor();
    TelemetryIngestor(const TelemetryIngestor &) = delete;
    TelemetryIngestor &operator=(const TelemetryIngestor &) = delete;
    TelemetryIngestor(TelemetryIngestor &&other) = delete;
    TelemetryIngestor &operator=(TelemetryIngestor &&other) = delete;

    TelemetryIngestorStartResult run();

    void stop();

private:
    bool running_{false};
    std::unique_ptr<lib::AfUnixUdpSocket> afUnixSock_;
    std::unique_ptr<lib::SPSCQueue<telemetry::TelemetryPacket>> spscQueue_;
    lib::SPSCQueueStats spscQueueStats_{};

    TelemetryIngestorStartResult init();

    void consume();
};

} // namespace app

#endif