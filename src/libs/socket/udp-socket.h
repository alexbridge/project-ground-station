
#ifndef LIB_SOCKET_UDP_H
#define LIB_SOCKET_UDP_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <optional>

namespace lib
{

#define UDP_SOCKET_STATE(X) \
    X(INITIAL)              \
    X(PRECONDITIONS)        \
    X(NO_SOCK)              \
    X(NO_SOCK_CONNECT)      \
    X(NO_SOCK_BIND)         \
    X(READY)                \
    X(CONNECT)              \
    X(BIND)

enum class UdpSocketState {
#define GENERATE_ENUM_VALUE(v) v,
    UDP_SOCKET_STATE(GENERATE_ENUM_VALUE)
#undef GENERATE_ENUM_VALUE
};

constexpr const char *toString(UdpSocketState s)
{
    switch (s) {
#define GENERATE_CASE(v)    \
    case UdpSocketState::v: \
        return #v;
        UDP_SOCKET_STATE(GENERATE_CASE)
#undef GENERATE_CASE
        default:
            return "UNKNOWN";
    }
}

class UdpSocket
{
public:
    explicit UdpSocket(uint16_t sockPort)
        : sockPort_(sockPort){};

    // As client
    UdpSocketState connect();

    // As server

    UdpSocketState bind();

    int sockFd() const;

    const sockaddr_in &getSockAddr() const;

    uint16_t port() const;

    std::optional<int> actualBufSize() const;

    ~UdpSocket();
    UdpSocket(const UdpSocket &) = delete;
    UdpSocket &operator=(const UdpSocket &) = delete;
    UdpSocket(UdpSocket &&other) = delete;
    UdpSocket &operator=(UdpSocket &&other) = delete;

private:
    uint16_t sockPort_;
    int sockFd_ = -1;
    UdpSocketState state_{UdpSocketState::INITIAL};
    sockaddr_in sockAddrIn_{};
    std::optional<int> actualBufSize_{std::nullopt};

    UdpSocketState initialize();

    void close();

    void setActualBufSize(int option);
};

} // namespace lib

#endif