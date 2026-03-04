#include "hpp/telemetry-server.hpp"
#include <atomic>
#include <csignal>

std::atomic<bool> running{true};

int main() {
  struct sigaction sa {};
  sa.sa_handler = signalHandler;
  sigaction(SIGINT, &sa, nullptr);

  return runServer();
}