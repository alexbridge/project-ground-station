

#include "apps/udp-to-af-unix-server/udp-to-af-unix-server.h"
#include "socket/udp-socket.h"
#include "telemetry-server/hpp/telemetry-server.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
namespace app
{

// extern std::atomic<bool> running;

UdpToAfUnixServer::UdpToAfUnixServer(uint16_t sockPort, std::string sockPath)
{
    lib::UdpSocket udpSock_{sockPort};
    lib::AfUnixUdpSocket afUnixSock_{std::move(sockPath)};
};

UdpToAfUnixServerStartResult UdpToAfUnixServer::init()
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

UdpToAfUnixServerStartResult UdpToAfUnixServer::run()
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

void UdpToAfUnixServer::stop()
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

UdpToAfUnixServer::~UdpToAfUnixServer()
{
}

} // namespace app