/* 
 * Copyright (C) Samuel Jones - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Samuel Jones <spjones329@gmail.com>, February 2024
 */
-- name: GetAirStat :one
SELECT
    *
FROM
    air_stats
WHERE
    id = $1
LIMIT
    1;

-- name: ListAirStats :many
SELECT
    *
FROM
    air_stats
ORDER BY
    created_at;

-- name: CreateAirStat :one
INSERT INTO
    air_stats (co2_ppm, temp_tick, humidity_tick)
VALUES
    ($1, $2, $3)
RETURNING
    *;

-- name: GetLatestAirStat :one
SELECT
    *
FROM
    air_stats
ORDER BY
    created_at DESC
LIMIT
    1;

-- name: GetAirStatsPerFiveMinutes :many
SELECT
    (
        '2010-01-01T00:00:00'::timestamp + interval '5 minutes' * (
            (
                (
                    DATE_PART(
                        'day',
                        T.created_at - '2010-01-01T00:00:00'::timestamp
                    ) * 24 + DATE_PART(
                        'hour',
                        T.created_at - '2010-01-01T00:00:00'::timestamp
                    )
                ) * 60 + DATE_PART(
                    'minute',
                    T.created_at - '2010-01-01T00:00:00'::timestamp
                )
            )::bigint / 5
        )
    )::timestamp AS five_minute_interval_start,
    AVG(T.co2_ppm)::int as co2_ppm,
    AVG(T.temp_tick)::int as temp_tick,
    AVG(T.humidity_tick)::int as humidity_tick
FROM
    air_stats T
GROUP BY
    five_minute_interval_start
ORDER BY
    five_minute_interval_start;