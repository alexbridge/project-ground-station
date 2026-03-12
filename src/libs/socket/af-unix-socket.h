
#ifndef LIB_SOCKET_AF_UNIX_H
#define LIB_SOCKET_AF_UNIX_H

#include <optional>
#include <string>
#include <sys/un.h>

namespace lib
{

#define AF_SOCKET_STATE(X) \
    X(INITIAL)             \
    X(SOCK_PRECONDITIONS)  \
    X(NO_SOCK)             \
    X(NO_SOCK_CONNECT)     \
    X(NO_SOCK_BIND)        \
    X(READY)               \
    X(CONNECT)             \
    X(BIND)

enum class AfUnixSocketState {
#define GENERATE_ENUM_VALUE(v) v,
    AF_SOCKET_STATE(GENERATE_ENUM_VALUE)
#undef GENERATE_ENUM_VALUE
};

constexpr const char *toString(AfUnixSocketState s)
{
    switch (s) {
#define GENERATE_CASE(v)       \
    case AfUnixSocketState::v: \
        return #v;
        AF_SOCKET_STATE(GENERATE_CASE)
#undef GENERATE_CASE
        default:
            return "UNKNOWN";
    }
}

class AfUnixUdpSocket
{
public:
    explicit AfUnixUdpSocket(std::string sockPath)
        : sockPath_(std::move(sockPath)){};

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