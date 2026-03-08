

#include "app/telemetry-api/telemetry-udp-api.h"
#include "socket/udp-socket.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

namespace app
{
TelemetryUdpApi::TelemetryUdpApi(
    uint16_t sockPort,
    std::string sockPath) : udpSock_{std::make_unique<lib::UdpSocket>(sockPort)},
                            afUnixSock_{std::make_unique<lib::AfUnixUdpSocket>(std::move(sockPath))} {};

TelemetryUdpApiRun TelemetryUdpApi::init()
{
    auto udpSockBindResult = udpSock_->bind();
    if (udpSockBindResult != lib::UdpSocketState::BIND) {
        return {
            false,
            std::string("UDP bind error: ") + lib::toString(udpSockBindResult)};
    }

    printf("UDP listen on port: %d\n", udpSock_->port());
    printf("UDP RCV actual buffer: %d bytes\n", udpSock_->actualBufSize().value());

    auto afUnixConnectResult = afUnixSock_->connect();
    if (afUnixConnectResult != lib::AfUnixSocketState::CONNECT) {
        return {
            false,
            std::string("AF-UNIX connect error: ") + lib::toString(afUnixConnectResult)};
    }

    return {
        true,
        std::string("OK")};
}

TelemetryUdpApiRun TelemetryUdpApi::run()
{
    auto in = init();
    if (!in.ok) {
        return in;
    };

    int udpSockFd = udpSock_->sockFd();
    int afUnixSockFd = afUnixSock_->sockFd();

    if (udpSock_ < 0 || afUnixSock_ < 0) {
        throw std::logic_error("Run init to initialize sockets");
    }

    std::byte raw_buf[1024];

    size_t sent = 0;

    while (running_) {
        ssize_t n = recvfrom(udpSockFd, raw_buf, sizeof(raw_buf), 0, nullptr, nullptr);
        if (n <= 0) {
            continue;
        }

        // natural: zero, positive
        size_t nat = static_cast<size_t>(n);

        send(afUnixSockFd, reinterpret_cast<const char *>(raw_buf), nat, 0);

        sent += nat;

        if (sent % 1024 == 0) {
            std::cout << "." << std::flush;
        }
    }

    std::cout.flush();

    return {
        true,
        std::string("OK")};
}

void TelemetryUdpApi::stop()
{
    running_ = false;
    auto udpSock = udpSock_->sockFd();
    if (udpSock > -1) {
        shutdown(udpSock, SHUT_RDWR);
    }
    auto afUnix = afUnixSock_->sockFd();
    if (afUnix > -1) {
        shutdown(afUnix, SHUT_RDWR);
    }
}

TelemetryUdpApi::~TelemetryUdpApi()
{
}

} // namespace app