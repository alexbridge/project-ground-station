#include <cstdio>

#include "telemetry-udp-api.h"

static app::TelemetryUdpApi *telemetryUdpApiPtr = nullptr;

void signalHandler(int sig)
{
    if (telemetryUdpApiPtr) {
        telemetryUdpApiPtr->stop();
    }
}

int main()
{
    struct sigaction sa {
    };
    sa.sa_handler = signalHandler;
    sigaction(SIGINT, &sa, nullptr);

    app::TelemetryUdpApi telemetryUdpApi{
        5005,
        app::TELEMETRY_SOCK_PATH};

    telemetryUdpApiPtr = &telemetryUdpApi;

    auto run = telemetryUdpApi.run();
    if (!run.ok) {
        printf("Error running server: %s", run.msg.c_str());
        return 1;
    };

    return 0;
}