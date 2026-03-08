#include "apps/udp-to-af-unix-server/udp-to-af-unix-server.h"
#include "telemetry-server/hpp/telemetry.hpp"
#include <csignal>
#include <cstdio>
#include <memory>

static app::UdpToAfUnixServer *serverPtr = nullptr;

void signalHandler(int sig)
{
    if (serverPtr) {
        serverPtr->stop();
    }
}

int main()
{
    struct sigaction sa {
    };
    sa.sa_handler = signalHandler;
    sigaction(SIGINT, &sa, nullptr);

    app::UdpToAfUnixServer server{
        5005,
        TELEMETRY_SOCK_PATH};

    serverPtr = &server;

    auto run = server.run();
    if (!run.ok) {
        printf("Error running server: %s", run.msg.c_str());
        return 1;
    };

    return 0;
}