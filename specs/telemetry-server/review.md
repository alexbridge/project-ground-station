# Code Review: src/telemetry-server/

## telemetry-server.cpp (main)

- [ ] **[High]** `std::signal` is not safe for multithreaded programs (line 9). The C++ standard says signal handlers in multithreaded programs have UB unless they only write to `volatile sig_atomic_t` or lock-free atomics. Handler calls `shutdownServer()` which calls `shutdown()` — a POSIX syscall inside a signal handler is technically unsafe. Use `sigaction` instead.
- [ ] **[Low]** `running` defined in .cpp, declared `extern` in .hpp — works, but the definition lives in `telemetry-server.cpp` which means you can only have one main. Fine for this project.

## hpp/telemetry-server.hpp

- [ ] **[High]** `signalHandler` is `inline` in a header (line 17). Signal handlers should have external linkage and a single definition. With `inline`, each TU gets its own copy. If the linker picks a different one, the address passed to `std::signal` may differ from what you expect. Make it non-inline in a .cpp.
- [ ] **[Medium]** `readPacket` is `inline` in a header (line 26). This is a non-trivial function with a loop and syscalls. Better in a .cpp. `inline` in headers is for tiny functions.
- [ ] **[Medium]** `readPacket` doesn't distinguish EOF from error (line 36). `n == 0` means peer closed connection (clean disconnect). `n < 0` means error (could be `EINTR`). Both return `false`. Handle `EINTR` with a retry and log errors vs clean disconnects differently.
- [ ] **[Low]** Heavy includes for a header — `<atomic>`, `<csignal>`, `<unistd.h>`, `<sys/socket.h>`, `<cstddef>` all pulled into every TU. Only `telemetry.hpp` and the function declarations are needed here. Move the rest to .cpp files.

## hpp/telemetry.hpp

- [ ] **[High]** Misaligned access in `printPacket` and `printAscii` (lines 32-50). Same issue fixed in storage. `packet.timestamp` and `packet.batteryV` are misaligned due to `pack(1)`. `printPacket` reads `packet.batteryV` directly (line 37) — needs `memcpy` like in the storage flush.
- [ ] **[Medium]** `hton`/`ntoh` modify packed fields in-place (lines 52-74). Writing to misaligned fields via assignment (`packet.appId = htons(...)`) is also UB with `pack(1)`. Works on x86 but not portable. Safer to `memcpy` in/out.
- [ ] **[Medium]** `<sstream>` included but unused (line 7). Remove.
- [ ] **[Medium]** `<atomic>` included but unused (line 5). Remove.
- [ ] **[Low]** `TELEMETRY_SOCK_PATH` is `constexpr static const char *` (line 15). `constexpr` on a pointer means the pointer itself is const, not the string. Use `inline constexpr const char *`. Also `static` in a header means each TU gets its own copy.
- [ ] **[Low]** Include guard name `CLANG_TELEMETRY` — rename to `GROUND_TELEMETRY_HPP` for consistency with the storage header.

## spsc-queue/spsc-queue.hpp

- [ ] **[High]** `SPSCQueueStats` has plain `size_t` fields accessed from two threads (lines 13-18). Producer writes `queue_pushed`/`queue_dropped`, consumer writes `queue_read`. This is a data race (UB). Use `std::atomic<size_t>` or split into per-thread structs.
- [ ] **[Medium]** Queue capacity is `Size - 1` (wastes one slot to distinguish full from empty). Standard trade-off, but worth documenting. A 1024-slot queue holds 1023 items max.
- [ ] **[Medium]** `cacheLine_` has a trailing underscore and is `static` in a header (lines 8-11). Each TU gets its own copy. Use `inline constexpr`. The trailing underscore conventionally means private member, confusing at namespace scope.
- [ ] **[Low]** Include guard `CLANG_TELEMETRY_SPSC` — rename for consistency.

## socket-telemetry-server.cpp

- [ ] **[Critical]** `#include <pqxx/pqxx>` not needed (line 9). This file doesn't use pqxx directly. Remove it — heavy header that slows compilation.
- [ ] **[High]** `socketFd` is a global `atomic<int>` (line 16). Shared mutable state between `runServer()` and `shutdownServer()`. If you ever have two server instances, they'll collide.
- [ ] **[High]** `clients` vector is unused (line 76). Dead code. Remove.
- [ ] **[High]** Single-client accept loop (lines 85-120). The inner `while (running)` at line 97 blocks the outer accept loop. Only one client at a time. If intentional (SPSC), add a comment.
- [ ] **[High]** `listen(sock_fd, 1)` backlog of 1 (line 68). Combined with single-client loop, a second client connecting may get `ECONNREFUSED`. Fine if single-client is by design.
- [ ] **[High]** No RAII for socket fd. If `bind` or `listen` fails, cleanup is manual. Use a SocketGuard wrapper.
- [ ] **[High]** `queueStats` accessed from two threads without synchronization (lines 33, 110-112). Producer writes `queue_pushed`/`queue_dropped`, consumer writes `queue_read`. Plain `size_t` fields — data race (UB).
- [ ] **[Medium]** `printAscii` + `printPacket` on every packet (lines 106-107). Will hammer stdout in production. Use a `constexpr bool DEBUG_PACKETS` flag.
- [ ] **[Medium]** Stray semicolon after `consume()` (line 45). `};` after function body.
- [ ] **[Medium]** C-style cast `(sockaddr *)` (line 62). Use `reinterpret_cast<sockaddr *>(&addr)`.
- [ ] **[Medium]** `sockStats` not reset between clients (line 80, 116-117). Stats accumulate across all clients. Log says "Client X stats" but shows cumulative values.
- [ ] **[Low]** Typo in comment (line 38). "cyles" should be "cycles".

## udp-telemetry-server.cpp

- [ ] **[Critical]** `send()` return value ignored (line 81). If the unix socket buffer is full or peer disconnected, `send()` returns `-1` and data is silently lost. Check the return value.
- [ ] **[Critical]** No validation that received UDP data is a complete packet (line 75-81). UDP datagrams are forwarded blindly. If truncated or malformed (not a multiple of `sizeof(TelemetryPacket)`), garbage enters the pipeline. Validate `n % sizeof(TelemetryPacket) == 0`.
- [ ] **[High]** `SO_REUSEADDR` on a Unix domain socket (line 23). Has no effect on `AF_UNIX` sockets — it's a TCP/IP concept. Dead code.
- [ ] **[High]** `SO_SNDBUF` set to 100 bytes (line 25-26). `10 * 10 = 100`. Comment says "10 packet at once" but math is `10 * 10`. Use `10 * sizeof(TelemetryPacket)` explicitly.
- [ ] **[High]** Leaks `sock_fd` if UDP socket creation fails (lines 46-51). Returns without closing `sock_fd`. RAII wrapper would fix this.
- [ ] **[Medium]** `printf` mixed with `std::cout` (line 31 vs 67). Pick one. Mixing can cause interleaved output due to separate buffers.
- [ ] **[Medium]** `sent % 1024` progress check (line 87). Works only if every `send` aligns to 1024. With variable UDP datagram sizes, the log could be skipped entirely.
- [ ] **[Low]** Magic port `5005` (line 56). Hardcoded. Make it a constant.
- [ ] **[Low]** Both `socketFd` and `udpFd` are globals (lines 10-11). Same concern as the socket server.

## Summary

| Severity | Count |
|----------|-------|
| Critical | 3 |
| High | 12 |
| Medium | 13 |
| Low | 9 |
