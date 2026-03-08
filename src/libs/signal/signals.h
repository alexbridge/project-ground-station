#ifndef LIB_SIGNALS_H
#define LIB_SIGNALS_H

#include "telemetry.hpp"
#include <atomic>
#include <csignal>
#include <cstddef>
#include <sys/socket.h>
#include <unistd.h>

extern std::atomic<bool> running;

int runServer();

void shutdownServer();

inline void signalHandler(int signal)
{
    if (signal == SIGINT) {
        running = false;
        shutdownServer();
    }
}

#endif