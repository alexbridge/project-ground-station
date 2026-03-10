#ifndef LIB_SIGNALS_H
#define LIB_SIGNALS_H

#include <atomic>
#include <csignal>

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