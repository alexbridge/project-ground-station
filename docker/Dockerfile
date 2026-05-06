FROM alpine:3.18 as builder

RUN apk update && apk add --no-cache cmake make clang musl-dev libstdc++ curl busybox-extras libpq-dev git

WORKDIR /build
COPY . .

RUN make clean && make init && make build

# ----------------------------------
FROM alpine:3.18 AS runtime-base

RUN apk add --no-cache libstdc++ libpq && \
    addgroup -g 1000 appgroup && \
    adduser -u 1000 -G appgroup -S -H appuser && \
    mkdir -p /opt/app /opt/sock && \
    chown appuser:appgroup /opt/app /opt/sock

WORKDIR /opt/app

# ----------------------------------
FROM runtime-base AS telemetry_ingestor
COPY --from=builder --chown=appuser:appgroup /build/build/src/app/telemetry_ingestor ./telemetry_ingestor
USER appuser
CMD ["./telemetry_ingestor"]

# ----------------------------------
FROM runtime-base AS telemetry_api
COPY --from=builder --chown=appuser:appgroup /build/build/src/app/telemetry_api ./telemetry_api
USER appuser
CMD ["./telemetry_api"]
