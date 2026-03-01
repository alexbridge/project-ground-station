SELECT * FROM TELEMETRY WHERE app_id = '24270' LIMIT 10;

SELECT COUNT(*) FROM TELEMETRY;

-- TRUNCATE FROM TELEMETRY;

-- Check total rows and table size
SELECT 
    count(*) AS total_rows,
    pg_size_pretty(pg_total_relation_size('TELEMETRY')) AS total_disk_usage
FROM TELEMETRY;


-- Calculate ingestion rate over the last 10 seconds
SELECT 
    COUNT(*) / 10.0 AS rows_per_second,
    COUNT(*) AS total_in_window,
    MIN(time) AS window_start,
    MAX(time) AS window_end
FROM TELEMETRY
WHERE time > NOW() - INTERVAL '5 seconds';


-- This built-in function is the most reliable way to check size
SELECT * FROM hypertable_detailed_size('TELEMETRY');


