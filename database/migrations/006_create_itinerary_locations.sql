CREATE TABLE itinerary_locations (
    id               BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    itinerary_day_id BIGINT        NOT NULL REFERENCES itinerary_days (id) ON DELETE CASCADE,
    name             VARCHAR(150)  NOT NULL,
    description      TEXT,
    category         VARCHAR(30)   NOT NULL,
    estimated_cost   NUMERIC(12,2) NOT NULL DEFAULT 0,
    latitude         NUMERIC(9,6),
    longitude        NUMERIC(9,6),
    CONSTRAINT itinerary_locations_name_not_blank CHECK (btrim(name) <> ''),
    CONSTRAINT itinerary_locations_category_allowed CHECK (
        category IN ('adventure', 'food', 'culture', 'history', 'nature',
                     'shopping', 'nightlife', 'photography', 'sightseeing', 'other')
    ),
    CONSTRAINT itinerary_locations_cost_non_negative CHECK (estimated_cost >= 0),
    CONSTRAINT itinerary_locations_latitude_range CHECK (latitude BETWEEN -90 AND 90),
    CONSTRAINT itinerary_locations_longitude_range CHECK (longitude BETWEEN -180 AND 180),
    CONSTRAINT itinerary_locations_coordinates_pair CHECK ((latitude IS NULL) = (longitude IS NULL))
);

COMMENT ON TABLE itinerary_locations IS 'Recommended places for an itinerary day, with estimated cost in INR.';

CREATE INDEX itinerary_locations_day_id_idx ON itinerary_locations (itinerary_day_id);
