#include "socket/udp-socket.h"

#include <cstdint>
#include <sys/socket.h>
#include <unistd.h>

#include "udp-socket.h"

namespace lib
{

UdpSocketState UdpSocket::bind()
{
    // Prev applicable state
    close();

    state_ = initialize();
    if (state_ != UdpSocketState::READY) {
        return state_;
    }

    int reuse{1};
    setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr *readdr = reinterpret_cast<sockaddr *>(&sockAddrIn_);
    if (::bind(sockFd_, readdr, sizeof(sockAddrIn_)) < 0) {
        state_ = UdpSocketState::NO_SOCK_BIND;
        return state_;
    }

    setActualBufSize(SO_RCVBUF);

    state_ = UdpSocketState::BIND;
    return state_;
}

UdpSocketState UdpSocket::connect()
{
    // Prev applicable state
    close();

    state_ = initialize();
    if (state_ != UdpSocketState::READY) {
        return state_;
    }

    sockaddr *readdr = reinterpret_cast<sockaddr *>(&sockAddrIn_);
    if (::connect(sockFd_, readdr, sizeof(sockAddrIn_)) < 0) {
        state_ = UdpSocketState::NO_SOCK_CONNECT;
        return state_;
    }

    setActualBufSize(SO_RCVBUF);

    state_ = UdpSocketState::CONNECT;
    return state_;
}

UdpSocketState UdpSocket::initialize()
{
    if (sockPort_ == 0) {
        return UdpSocketState::PRECONDITIONS;
    }

    sockFd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd_ < 0) {
        return UdpSocketState::NO_SOCK;
    }

    sockAddrIn_.sin_family = AF_INET;
    sockAddrIn_.sin_addr.s_addr = INADDR_ANY;
    sockAddrIn_.sin_port = htons(sockPort_);

    return UdpSocketState::READY;
}

void UdpSocket::setActualBufSize(int option)
{
    int actual{};
    socklen_t len = sizeof(actual);
    getsockopt(sockFd_, SOL_SOCKET, option, &actual, &(len));
    actualBufSize_ = actual;
}

int UdpSocket::sockFd() const
{
    return sockFd_;
}

std::optional<int> UdpSocket::actualBufSize() const
{
    return actualBufSize_;
}

uint16_t UdpSocket::port() const
{
    return sockPort_;
}

UdpSocket::~UdpSocket()
{
    close();
}

void UdpSocket::close()
{
    if (sockFd_ >= 0) {
        ::close(sockFd_);
        sockFd_ = -1;
    }
    state_ = UdpSocketState::INITIAL;
}

const sockaddr_in &UdpSocket::getSockAddr() const
{
    return sockAddrIn_;
}
} // namespace lib
