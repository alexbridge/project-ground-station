#include <csignal>
#include <atomic>
#include "hpp/telemetry-server.hpp"

std::atomic<bool> running(true);

int main()
{
    std::signal(SIGINT, signalHandler);

    return runServer();
}