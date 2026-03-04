#include "hpp/telemetry-server.hpp"
#include "hpp/telemetry.hpp"
#include "spsc-queue/spsc-queue.hpp"
#include "storage/telemetry-storage.hpp"
#include <atomic>
#include <csignal>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

constexpr bool DEBUG_PACKETS = false;

std::atomic<int> socketFd{-1};

struct SocketGuard {
  int fd;
  explicit SocketGuard(int fd) : fd(fd) {}
  ~SocketGuard() {
    if (fd >= 0) {
      close(fd);
      unlink(TELEMETRY_SOCK_PATH);
    }
  }
  SocketGuard(const SocketGuard &) = delete;
  SocketGuard &operator=(const SocketGuard &) = delete;
};

void consume(SPSCQueue<TelemetryPacket> &spscQueue,
             SPSCQueueStats &queueStats) {
  TelemetryPacket packet{};
  TelemetryStorage telemetryStorage{};

  while (running) {
    // 1. Ensure connection is active
    if (!telemetryStorage.connected()) {
      if (!telemetryStorage.connect()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }
    }

    if (spscQueue.pop(packet)) {
      queueStats.queue_read++;
      telemetryStorage.add(packet);
    } else {
      // If we've been idle, flush partial batch
      telemetryStorage.flush();
      // Release cpu cyles
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
  }

  // Flush leftover
  telemetryStorage.flush();
};

int runServer() {
  int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("Error creating socket.");
    return 1;
  }

  SocketGuard guard(sock_fd);

  socketFd = sock_fd;

  unlink(TELEMETRY_SOCK_PATH);

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, TELEMETRY_SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (bind(sock_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    perror("bind error");
    return 1;
  }

  if (listen(sock_fd, 1) < 0) {
    perror("listen error");
    return 1;
  }

  std::cout << "Listening on " << TELEMETRY_SOCK_PATH << std::endl;

  SPSCQueue<TelemetryPacket> spscQueue{};
  SPSCQueueStats queueStats{};
  ServerSockStats sockStats{};

  std::thread consumer(
      [&spscQueue, &queueStats]() { consume(spscQueue, queueStats); });

  while (running) {
    int client_fd = accept(sock_fd, nullptr, nullptr);
    if (client_fd < 0) {
      if (!running) {
        break;
      }
      perror("accept error");
      break;
    }

    std::cout << "Client connected!" << std::endl;

    while (running) {
      TelemetryPacket packet{};
      if (!readPacket(client_fd, packet, sockStats)) {
        break;
      }

      // Net to host
      ntoh(packet);

      if constexpr (DEBUG_PACKETS) {
        printAscii(packet);
        printPacket(packet);
      }

      if (spscQueue.push(packet)) {
        queueStats.queue_pushed++;
      } else {
        queueStats.queue_dropped++;
      }
    }

    std::cout << "Client " << client_fd << " stats: reads " << sockStats.reads
              << " stats: partials " << sockStats.partial_reads << std::endl;

    close(client_fd);
  }

  consumer.join();
  std::cout.flush();

  return 0;
}

void shutdownServer() {
  if (socketFd > -1) {
    shutdown(socketFd, SHUT_RDWR);
  }
}