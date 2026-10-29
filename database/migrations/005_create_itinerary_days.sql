CREATE TABLE itinerary_days (
    id          BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    trip_id     BIGINT       NOT NULL REFERENCES trips (id) ON DELETE CASCADE,
    day_number  INTEGER      NOT NULL,
    date        DATE,
    title       VARCHAR(150) NOT NULL,
    description TEXT,
    -- Also serves as the index for reading a trip's days in order.
    CONSTRAINT itinerary_days_trip_day_key UNIQUE (trip_id, day_number),
    CONSTRAINT itinerary_days_day_number_positive CHECK (day_number >= 1),
    CONSTRAINT itinerary_days_title_not_blank CHECK (btrim(title) <> '')
);

COMMENT ON TABLE itinerary_days IS 'Day-wise plan for a trip (day 1, day 2, ...).';
