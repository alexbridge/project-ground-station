#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <iostream>
#include "hpp/telemetry-server.hpp"
#include "hpp/telemetry.hpp"
#include <cstddef>

std::atomic<int> socketFd{-1};
std::atomic<int> udpFd{-1};

int runServer()
{
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket start error");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, TELEMETRY_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    socketFd.store(sock_fd);

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0)
    {
        perror("Error creating udp socket.");
        return 1;
    }

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(5005);

    if (bind(udp_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("udp bind error");
        close(udp_fd);
        return 1;
    }

    udpFd.store(udp_fd);

    std::cout << "UDP listen on " << udp_fd << std::endl;

    std::byte raw_buf[1024];

    size_t sent = 0;

    while (running)
    {
        ssize_t n = read(udp_fd, raw_buf, sizeof(raw_buf));
        if (n <= 0)
        {
            continue;
        }

        send(sock_fd, reinterpret_cast<const char *>(raw_buf), static_cast<size_t>(n), 0);

        sent += static_cast<size_t>(n);

        std::cout << ".";

        if (sent % 1024 == 0)
        {
            std::cout << " .1024. " << sent << std::endl;
        }
    }

    std::cout.flush();

    close(udp_fd);
    close(sock_fd);

    return 0;
}

void shutdownServer()
{
    if (socketFd > -1)
    {
        shutdown(socketFd, SHUT_RDWR);
    }
    if (udpFd > -1)
    {
        shutdown(udpFd, SHUT_RDWR);
    }
}