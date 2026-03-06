
#ifndef LIB_SOCKET_UDP_H
#define LIB_SOCKET_UDP_H

#include <cstdint>
#include <netinet/in.h>
#include <optional>

namespace lib
{

enum class UdpSocketState { INITIAL, PRECONDITIONS, NO_SOCK, NO_SOCK_CONNECT, NO_SOCK_BIND, READY, CONNECT, BIND };

class UdpSocket
{
public:
    explicit UdpSocket(uint16_t sockPort) : sockPort_(sockPort){};

    // As client
    UdpSocketState connect();

    // As server
    UdpSocketState bind();

    int sockFd() const;

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