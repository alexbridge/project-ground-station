#ifndef LIB_LOGGER_H
#define LIB_LOGGER_H

#include <memory>
#include <string>

#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace lib
{
class Logger
{
public:
    static void init()
    {
        spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] [%n] %v");
    }

    static std::shared_ptr<spdlog::logger> get(const std::string &name)
    {
        auto logger = spdlog::get(name);
        if (!logger) {
            logger = spdlog::stdout_color_mt(name);
        }
        return logger;
    }
};

} // namespace lib

#endif