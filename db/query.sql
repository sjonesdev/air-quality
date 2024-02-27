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