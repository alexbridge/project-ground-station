#include <csignal>
#include <cstdio>
#include <fstream>
#include <memory>

#include "app-commons.h"
#include "logging/logger.h"
#include "spdlog/logger.h"
#include "telemetry-ingestor/telemetry-ingestor.h"

constexpr const char *APP_STATUS_PATH = "/opt/app/status";

static app::TelemetryIngestor         *telemetryIngestorPtr = nullptr;
static std::shared_ptr<spdlog::logger> mainLogger           = nullptr;

void signalHandler(int sig) {
    mainLogger->info("SIG {} stop", sig);

    if (telemetryIngestorPtr) {
        telemetryIngestorPtr->stop();
    }
}

void healthStatus(std::string status) { std::ofstream(APP_STATUS_PATH) << status; }

int main() {
    mainLogger = lib::Logger::get("TelemetryIngestorApiMain");

    healthStatus("RUNNING");

    struct sigaction sa {};
    sa.sa_handler = signalHandler;
    sigaction(SIGINT, &sa, nullptr);

    mainLogger->info("Preparing ingestor ..");

    app::TelemetryIngestor telemetryIngestor{app::TELEMETRY_SOCK_PATH};

    mainLogger->info("Ingestor prepared");

    telemetryIngestorPtr = &telemetryIngestor;

    mainLogger->info("Starting ingestor ...");
    auto run = telemetryIngestor.run();
    if (!run.ok) {
        mainLogger->error("Error running server: {}", run.msg);
        return 1;
    } else {
        mainLogger->info(run.msg);
    }

    return 0;
}