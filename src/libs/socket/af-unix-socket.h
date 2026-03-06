
#ifndef LIB_SOCKET_AF_UNIX_H
#define LIB_SOCKET_AF_UNIX_H

#include <optional>
#include <string>
#include <sys/un.h>

namespace lib
{

enum class AfUnixSocketState {
    INITIAL,
    SOCK_PRECONDITIONS,
    NO_SOCK,
    NO_SOCK_CONNECT,
    NO_SOCK_BIND,
    READY,
    CONNECT,
    BIND
};

class AfUnixUdpSocket
{
public:
    explicit AfUnixUdpSocket(std::string sockPath) : sockPath_(std::move(sockPath)){};

    // As client
    AfUnixSocketState connect();

    // As server
    AfUnixSocketState bind();

    int sockFd() const;

    std::optional<int> actualBufSize() const;

    ~AfUnixUdpSocket();
    AfUnixUdpSocket(const AfUnixUdpSocket &) = delete;
    AfUnixUdpSocket &operator=(const AfUnixUdpSocket &) = delete;
    AfUnixUdpSocket(AfUnixUdpSocket &&other) = delete;
    AfUnixUdpSocket &operator=(AfUnixUdpSocket &&other) = delete;

private:
    std::string sockPath_;
    int sockFd_ = -1;
    std::optional<AfUnixSocketState> state_{AfUnixSocketState::INITIAL};
    ::sockaddr_un sockAddr_{};
    std::optional<int> actualBufSize_{std::nullopt};

    AfUnixSocketState initializeSock();

    void close();

    void setActualBufSize(int option);
};

} // namespace lib

#endif