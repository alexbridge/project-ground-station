#ifndef LIB_UTILS_ENV_H
#define LIB_UTILS_ENV_H

#include <cstdlib>

namespace lib {

const char* getEnv(const char* key, const char* defaultValue) {
    const char* envValue = std::getenv(key);
    return envValue ? envValue : defaultValue;
}

}  // namespace lib

#endif