#ifndef APP_AF_UNIX_TELEMETRY_SERVER_H
#define APP_AF_UNIX_TELEMETRY_SERVER_H

#include "socket/af-unix-socket.h"
#include "socket/udp-socket.h"
#include "spsc-queue/spsc-queue.hpp"
#include "telemetry-server/hpp/telemetry.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace app
{

struct AfUnixTelemetryServerStartResult {
    bool ok;
    std::string msg;
};

class AfUnixTelemetryServer
{
public:
    explicit AfUnixTelemetryServer(std::string sockPath);

    ~AfUnixTelemetryServer();
    AfUnixTelemetryServer(const AfUnixTelemetryServer &) = delete;
    AfUnixTelemetryServer &operator=(const AfUnixTelemetryServer &) = delete;
    AfUnixTelemetryServer(AfUnixTelemetryServer &&other) = delete;
    AfUnixTelemetryServer &operator=(AfUnixTelemetryServer &&other) = delete;

    AfUnixTelemetryServerStartResult run();

    void stop();

private:
    bool running_{false};
    std::unique_ptr<lib::AfUnixUdpSocket> afUnixSock_{nullptr};
    std::unique_ptr<lib::SPSCQueue<TelemetryPacket>> spscQueue_{};

    AfUnixTelemetryServerStartResult init();

    void consume();
};

} // namespace app

#endif