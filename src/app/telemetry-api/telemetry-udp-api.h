#ifndef APP_UDP_API_H
#define APP_UDP_API_H

#include "socket/af-unix-socket.h"
#include "socket/udp-socket.h"
#include <cstdint>
#include <memory>
#include <string>

namespace app
{

struct TelemetryUdpApiRun {
    bool ok;
    std::string msg;
};

class TelemetryUdpApi
{
public:
    explicit TelemetryUdpApi(uint16_t sockPort, std::string sockPath);

    ~TelemetryUdpApi();
    TelemetryUdpApi(const TelemetryUdpApi &) = delete;
    TelemetryUdpApi &operator=(const TelemetryUdpApi &) = delete;
    TelemetryUdpApi(TelemetryUdpApi &&other) = delete;
    TelemetryUdpApi &operator=(TelemetryUdpApi &&other) = delete;

    TelemetryUdpApiRun run();

    void stop();

private:
    bool running_{false};
    std::unique_ptr<lib::UdpSocket> udpSock_{nullptr};
    std::unique_ptr<lib::AfUnixUdpSocket> afUnixSock_{nullptr};

    TelemetryUdpApiRun init();

    static std::shared_ptr<spdlog::logger> &log()
    {
        static auto instance = Log::get("Connection");
          // created once
            return instance;
    }
};

} // namespace app

#endif