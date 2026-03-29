FROM alpine:3.18 as builder

RUN apk update && apk add --no-cache cmake make clang musl-dev libstdc++ curl busybox-extras libpq-dev git

WORKDIR /build
COPY . .

RUN make clean && make init && make build

FROM alpine:3.18 AS telemetry_ingestor

RUN apk add --no-cache libstdc++ libpq && \
    addgroup -g 1000 appgroup && \
    adduser -u 1000 -G appgroup -S -H appuser

RUN mkdir -p /opt && chown appuser:appgroup /opt
WORKDIR /opt/app
COPY --from=builder --chown=appuser:appgroup /build/build/src/app/telemetry_ingestor ./app
RUN chmod +x ./app
USER appuser
CMD ["./app"]

FROM alpine:3.18 AS telemetry_api

RUN apk add --no-cache libstdc++ libpq && \
    addgroup -g 1000 appgroup && \
    adduser -u 1000 -G appgroup -S -H appuser

RUN mkdir -p /opt && chown appuser:appgroup /opt
WORKDIR /opt/app
COPY --from=builder --chown=appuser:appgroup /build/build/src/app/telemetry_api ./app
RUN chmod +x ./app
USER appuser
CMD ["./app"]
