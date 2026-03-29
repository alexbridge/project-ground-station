#include <unistd.h>

#include <csignal>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "app-commons.h"
#include "logging/logger.h"
#include "spdlog/logger.h"
#include "telemetry-api/telemetry-udp-api.h"

constexpr const char *APP_STATUS_PATH = "/opt/app/status";

static app::TelemetryUdpApi           *telemetryUdpApiPtr = nullptr;
static std::shared_ptr<spdlog::logger> mainLogger         = nullptr;

void signalHandler(int sig) {
    mainLogger->info("SIG {} stop", sig);

    if (telemetryUdpApiPtr) {
        telemetryUdpApiPtr->stop();
        unlink(APP_STATUS_PATH);
    }
}

void healthStatus(std::string status) { std::ofstream(APP_STATUS_PATH) << status; }

int main() {
    mainLogger = lib::Logger::get("TelemetryUdpApiMain");

    healthStatus("RUNNING");

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
        healthStatus("RUNNING");
        mainLogger->info(run.msg);
    }

    return 0;
}