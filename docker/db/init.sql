CREATE DATABASE "GROUND_STATION";

\c "GROUND_STATION"

CREATE EXTENSION IF NOT EXISTS timescaledb;

CREATE TABLE "TELEMETRY" (
    time        TIMESTAMPTZ       NOT NULL,
    app_id   INTEGER           NOT NULL,
    temperature DOUBLE PRECISION  NULL,
    voltage DOUBLE PRECISION  NULL
);

SELECT create_hypertable('"TELEMETRY"', 'time');

ALTER TABLE "TELEMETRY" SET (
  timescaledb.compress,
  timescaledb.compress_segmentby = 'app_id'
);

SELECT add_compression_policy('"TELEMETRY"', INTERVAL '7 days');

SELECT add_retention_policy('"TELEMETRY"', INTERVAL '90 days');