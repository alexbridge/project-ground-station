#ifndef APP_UDP_TO_AF_UNIX_SERVER_H
#define APP_UDP_TO_AF_UNIX_SERVER_H

#include "socket/af-unix-socket.h"
#include "socket/udp-socket.h"
#include <cstdint>
#include <memory>
#include <string>

namespace app
{

struct UdpToAfUnixServerStartResult {
    bool ok;
    std::string msg;
};

class UdpToAfUnixServer
{
public:
    explicit UdpToAfUnixServer(uint16_t sockPort, std::string sockPath);

    ~UdpToAfUnixServer();
    UdpToAfUnixServer(const UdpToAfUnixServer &) = delete;
    UdpToAfUnixServer &operator=(const UdpToAfUnixServer &) = delete;
    UdpToAfUnixServer(UdpToAfUnixServer &&other) = delete;
    UdpToAfUnixServer &operator=(UdpToAfUnixServer &&other) = delete;

    UdpToAfUnixServerStartResult run();

    void stop();

private:
    bool running_{false};
    std::unique_ptr<lib::UdpSocket> udpSock_{nullptr};
    std::unique_ptr<lib::AfUnixUdpSocket> afUnixSock_{nullptr};

    UdpToAfUnixServerStartResult init();
};

} // namespace app

#endif