#include "af-unix-socket.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

namespace lib {

AfUnixSocketState AfUnixUdpSocket::connect() {
    // Prev applicable state
    close();

    state_ = initializeSock();
    if (state_ != AfUnixSocketState::READY) {
        return state_.value();
    }

    auto *readdr = reinterpret_cast<sockaddr *>(&sockAddr_);
    if (::connect(sockFd_, readdr, sizeof(sockAddr_)) < 0) {
        state_ = AfUnixSocketState::NO_SOCK_CONNECT;
        return state_.value();
    }

    setActualBufSize(SO_RCVBUF);

    state_ = AfUnixSocketState::CONNECT;
    return state_.value();
}

AfUnixSocketState AfUnixUdpSocket::bind() {
    // Prev applicable state
    close();

    state_ = initializeSock();
    if (state_ != AfUnixSocketState::READY) {
        return state_.value();
    }

    auto *readdr = reinterpret_cast<sockaddr *>(&sockAddr_);
    if (::bind(sockFd_, readdr, sizeof(sockAddr_)) < 0) {
        state_ = AfUnixSocketState::NO_SOCK_BIND;
        return state_.value();
    }

    setActualBufSize(SO_SNDBUF);

    state_ = AfUnixSocketState::BIND;
    return state_.value();
}

AfUnixSocketState AfUnixUdpSocket::initializeSock() {
    if (sockPath_.size() > sizeof(sockAddr_.sun_path) - 1) {
        return AfUnixSocketState::SOCK_PRECONDITIONS;
    }

    sockFd_ = socket(AF_UNIX, SOCK_DGRAM, 0);

    log()->info("Af-unix SOCK_DGRAM sock-fd {}", sockFd_);

    if (sockFd_ < 0) {
        return AfUnixSocketState::NO_SOCK;
    }

    sockAddr_.sun_family = AF_UNIX;
    std::strncpy(sockAddr_.sun_path, sockPath_.c_str(), sizeof(sockAddr_.sun_path) - 1);

    log()->info("Af-unix SOCK_DGRAM sock-fd {} is READY", sockFd_);
    return AfUnixSocketState::READY;
}

void AfUnixUdpSocket::setActualBufSize(int option) {
    int       actual{};
    socklen_t len = sizeof(actual);
    getsockopt(sockFd_, SOL_SOCKET, option, &actual, &(len));
    actualBufSize_ = actual;
}

int AfUnixUdpSocket::sockFd() const { return sockFd_; }

std::optional<int> AfUnixUdpSocket::actualBufSize() const { return actualBufSize_; }

AfUnixUdpSocket::~AfUnixUdpSocket() { close(); }

void AfUnixUdpSocket::close() {
    if (sockFd_ >= 0) {
        ::close(sockFd_);
        sockFd_ = -1;
    }
    unlink(sockPath_.c_str());
    state_ = AfUnixSocketState::INITIAL;
}

}  // namespace lib
