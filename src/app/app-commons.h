#ifndef APP_COMMONS_H
#define APP_COMMONS_H

#include <cstddef>

namespace app {

constexpr static const char *TELEMETRY_SOCK_PATH = "/opt/sock/telemetry.sock";

struct ServerSockStats {
    size_t reads{0};
    size_t partial_reads{0};
};
}  // namespace app

#endif