#include <csignal>
#include <cstdio>
#include <memory>

#include "app-commons.h"
#include "logging/logger.h"
#include "spdlog/logger.h"
#include "telemetry-ingestor/telemetry-ingestor.h"

static app::TelemetryIngestor *telemetryIngestorPtr = nullptr;
static std::shared_ptr<spdlog::logger> mainLogger = nullptr;

void signalHandler(int sig)
{
    mainLogger->info("SIG {} stop", sig);

    if (telemetryIngestorPtr) {
        telemetryIngestorPtr->stop();
    }
}

int main()
{
    mainLogger = lib::Logger::get("TelemetryIngestorApiMain");

    struct sigaction sa {
    };
    sa.sa_handler = signalHandler;
    sigaction(SIGINT, &sa, nullptr);

    mainLogger->info("Preparing ..");

    app::TelemetryIngestor telemetryIngestor{app::TELEMETRY_SOCK_PATH};

    mainLogger->info("Prepared");

    telemetryIngestorPtr = &telemetryIngestor;

    mainLogger->info("Starting ...");
    auto run = telemetryIngestor.run();
    if (!run.ok) {
        mainLogger->error("Error running server: {}", run.msg);
        return 1;
    } else {
        mainLogger->info(run.msg);
    }

    return 0;
}