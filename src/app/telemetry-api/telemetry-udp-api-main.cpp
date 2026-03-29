#include <csignal>
#include <cstdio>
#include <memory>

#include "app-commons.h"
#include "logging/logger.h"
#include "spdlog/logger.h"
#include "telemetry-api/telemetry-udp-api.h"

static app::TelemetryUdpApi           *telemetryUdpApiPtr = nullptr;
static std::shared_ptr<spdlog::logger> mainLogger         = nullptr;

void signalHandler(int sig) {
    mainLogger->info("SIG {} stop", sig);

    if (telemetryUdpApiPtr) {
        telemetryUdpApiPtr->stop();
    }
}

int main() {
    mainLogger = lib::Logger::get("TelemetryUdpApiMain");

    struct sigaction sa {};
    sa.sa_handler = signalHandler;
    sigaction(SIGINT, &sa, nullptr);

    app::TelemetryUdpApi telemetryUdpApi{5005, app::TELEMETRY_SOCK_PATH};

    telemetryUdpApiPtr = &telemetryUdpApi;

    auto run = telemetryUdpApi.run();
    if (!run.ok) {
        mainLogger->error("Error running server: {}", run.msg);
        return 1;
    } else {
        mainLogger->info(run.msg);
    }

    return 0;
}