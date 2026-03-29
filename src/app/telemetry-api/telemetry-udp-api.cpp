

#include "telemetry-udp-api.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

namespace app {
TelemetryUdpApi::TelemetryUdpApi(uint16_t sockPort, std::string sockPath)
    : udpSock_{std::make_unique<lib::UdpSocket>(sockPort)},
      afUnixSock_{std::make_unique<lib::AfUnixUdpSocket>(std::move(sockPath))} {};

TelemetryUdpApiRun TelemetryUdpApi::init() {
    auto udpSockBindResult = udpSock_->bind();
    if (udpSockBindResult != lib::UdpSocketState::BIND) {
        return {false, fmt::format("UDP bind error: {}", lib::toString(udpSockBindResult))};
    }

    auto afUnixConnectResult = afUnixSock_->connect();
    if (afUnixConnectResult != lib::AfUnixSocketState::CONNECT) {
        return {false, fmt::format("AF-UNIX connect error: {}", lib::toString(afUnixConnectResult))};
    }

    return {
        true,
        fmt::format(
            "Telemetry UDP: port {}, RCV actual buffer {}bytes",
            udpSock_->port(),
            udpSock_->actualBufSize().value()
        )};
}

TelemetryUdpApiRun TelemetryUdpApi::run() {
    auto in = init();
    if (!in.ok) {
        return in;
    };

    int udpSockFd    = udpSock_->sockFd();
    int afUnixSockFd = afUnixSock_->sockFd();

    if (udpSock_ < 0 || afUnixSock_ < 0) {
        throw std::logic_error("Run init to initialize sockets");
    }

    running_ = true;

    size_t    sent = 0;
    std::byte raw_buf[1024];

    while (running_) {
        ssize_t n = recv(udpSockFd, raw_buf, sizeof(raw_buf), 0);
        if (n <= 0) {
            continue;
        }

        // natural: zero, positive
        size_t nat = static_cast<size_t>(n);
        log()->info("UDP Received {}", nat);

        send(afUnixSockFd, reinterpret_cast<const char *>(raw_buf), nat, 0);
        log()->info("UDP Send {}", nat);

        sent += nat;
    }

    return {true, fmt::format("OK, sent {}", sent)};
}

void TelemetryUdpApi::stop() {
    running_     = false;
    auto udpSock = udpSock_->sockFd();
    if (udpSock > -1) {
        shutdown(udpSock, SHUT_RDWR);
    }
    auto afUnix = afUnixSock_->sockFd();
    if (afUnix > -1) {
        shutdown(afUnix, SHUT_RDWR);
    }
}

TelemetryUdpApi::~TelemetryUdpApi() {}

}  // namespace app