/* 
 * Copyright (C) Samuel Jones - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Samuel Jones <spjones329@gmail.com>, February 2024
 */
CREATE TABLE
    air_stats (
        id bigserial PRIMARY KEY,
        co2_ppm int NOT NULL,
        -- Convert value to °C by: -45 °C + 175 °C * value/2^16
        temp_tick int NOT NULL,
        -- Convert value to %RH by: 100%RH * value/2^16
        humidity_tick int NOT NULL,
        created_at timestamp NOT NULL DEFAULT current_timestamp
    );

CREATE INDEX index_created_at ON air_stats (created_at);