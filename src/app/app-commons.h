#ifndef APP_COMMONS_H
#define APP_COMMONS_H

#include <cstddef>

#include "utils/env-utils.h"

namespace app {

inline const char *TELEMETRY_SOCK_PATH = lib::getEnv("TELEMETRY_AF_SOCK", "/tmp/telemetry.sock");

struct ServerSockStats {
    size_t reads{0};
    size_t partial_reads{0};
};
}  // namespace app

#endif