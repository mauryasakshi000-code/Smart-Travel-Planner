-- Phase 3+ product fields: dates, travel style, activities, richer itinerary
-- columns, saved places. Reuses users, trips, itinerary_*, expenses, blogs.

ALTER TABLE trips
    ADD COLUMN IF NOT EXISTS start_date DATE,
    ADD COLUMN IF NOT EXISTS end_date DATE,
    ADD COLUMN IF NOT EXISTS currency VARCHAR(3) NOT NULL DEFAULT 'INR',
    ADD COLUMN IF NOT EXISTS members INTEGER NOT NULL DEFAULT 1,
    ADD COLUMN IF NOT EXISTS travel_style VARCHAR(20) NOT NULL DEFAULT 'standard',
    ADD COLUMN IF NOT EXISTS accommodation_preference VARCHAR(80),
    ADD COLUMN IF NOT EXISTS transportation_preference VARCHAR(80),
    ADD COLUMN IF NOT EXISTS dietary_preferences VARCHAR(120),
    ADD COLUMN IF NOT EXISTS special_requirements TEXT,
    ADD COLUMN IF NOT EXISTS notes TEXT,
    ADD COLUMN IF NOT EXISTS status VARCHAR(20) NOT NULL DEFAULT 'planned';

ALTER TABLE trips DROP CONSTRAINT IF EXISTS trips_interests_allowed;
ALTER TABLE trips DROP CONSTRAINT IF EXISTS trips_duration_range;
ALTER TABLE trips DROP CONSTRAINT IF EXISTS trips_dates_order;
ALTER TABLE trips DROP CONSTRAINT IF EXISTS trips_members_positive;
ALTER TABLE trips DROP CONSTRAINT IF EXISTS trips_currency_format;
ALTER TABLE trips DROP CONSTRAINT IF EXISTS trips_travel_style_allowed;
ALTER TABLE trips DROP CONSTRAINT IF EXISTS trips_status_allowed;

ALTER TABLE trips
    ADD CONSTRAINT trips_duration_range CHECK (duration_days BETWEEN 1 AND 60),
    ADD CONSTRAINT trips_dates_order CHECK (end_date IS NULL OR start_date IS NULL OR end_date >= start_date),
    ADD CONSTRAINT trips_members_positive CHECK (members >= 1),
    ADD CONSTRAINT trips_currency_format CHECK (currency ~ '^[A-Z]{3}$'),
    ADD CONSTRAINT trips_travel_style_allowed CHECK (
        travel_style IN ('budget', 'standard', 'luxury', 'adventure')
    ),
    ADD CONSTRAINT trips_status_allowed CHECK (
        status IN ('draft', 'planned', 'completed', 'cancelled')
    ),
    ADD CONSTRAINT trips_interests_allowed CHECK (
        interests <@ ARRAY[
            'adventure', 'food', 'culture', 'history', 'nature', 'shopping',
            'nightlife', 'photography', 'sightseeing', 'historical_places',
            'museums', 'beaches', 'relaxation', 'family'
        ]::TEXT[]
    );

CREATE INDEX IF NOT EXISTS trips_user_id_start_date_idx ON trips (user_id, start_date);

CREATE TABLE IF NOT EXISTS trip_activities (
    id            BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    trip_id       BIGINT       NOT NULL REFERENCES trips (id) ON DELETE CASCADE,
    activity_name VARCHAR(40)  NOT NULL,
    CONSTRAINT trip_activities_trip_activity_key UNIQUE (trip_id, activity_name),
    CONSTRAINT trip_activities_name_allowed CHECK (
        activity_name IN (
            'adventure', 'food', 'culture', 'history', 'nature', 'shopping',
            'nightlife', 'photography', 'sightseeing', 'historical_places',
            'museums', 'beaches', 'relaxation', 'family'
        )
    )
);

CREATE INDEX IF NOT EXISTS trip_activities_trip_id_idx ON trip_activities (trip_id);

ALTER TABLE itinerary_days
    ADD COLUMN IF NOT EXISTS meals TEXT,
    ADD COLUMN IF NOT EXISTS accommodation TEXT,
    ADD COLUMN IF NOT EXISTS transportation TEXT,
    ADD COLUMN IF NOT EXISTS daily_budget NUMERIC(12,2) NOT NULL DEFAULT 0;

ALTER TABLE itinerary_days DROP CONSTRAINT IF EXISTS itinerary_days_daily_budget_non_negative;
ALTER TABLE itinerary_days
    ADD CONSTRAINT itinerary_days_daily_budget_non_negative CHECK (daily_budget >= 0);

ALTER TABLE itinerary_locations
    ADD COLUMN IF NOT EXISTS visit_time VARCHAR(40),
    ADD COLUMN IF NOT EXISTS sequence_number INTEGER NOT NULL DEFAULT 1,
    ADD COLUMN IF NOT EXISTS transportation_notes TEXT;

ALTER TABLE itinerary_locations DROP CONSTRAINT IF EXISTS itinerary_locations_category_allowed;
ALTER TABLE itinerary_locations
    ADD CONSTRAINT itinerary_locations_category_allowed CHECK (
        category IN (
            'adventure', 'food', 'culture', 'history', 'nature', 'shopping',
            'nightlife', 'photography', 'sightseeing', 'other', 'relaxation', 'family'
        )
    );

ALTER TABLE expenses DROP CONSTRAINT IF EXISTS expenses_category_allowed;
ALTER TABLE expenses
    ADD CONSTRAINT expenses_category_allowed CHECK (
        category IN ('food', 'transport', 'accommodation', 'activities', 'shopping', 'other', 'miscellaneous')
    );

ALTER TABLE blogs
    ADD COLUMN IF NOT EXISTS cover_image_url TEXT;

CREATE TABLE IF NOT EXISTS saved_places (
    id          BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    user_id     BIGINT       NOT NULL REFERENCES users (id) ON DELETE CASCADE,
    place_name  VARCHAR(150) NOT NULL,
    destination VARCHAR(150),
    notes       TEXT,
    created_at  TIMESTAMPTZ  NOT NULL DEFAULT now(),
    CONSTRAINT saved_places_place_not_blank CHECK (btrim(place_name) <> '')
);

CREATE INDEX IF NOT EXISTS saved_places_user_id_idx ON saved_places (user_id, created_at DESC);

COMMENT ON TABLE trip_activities IS 'Selected activity tags for a trip (many-to-many via rows).';
COMMENT ON TABLE saved_places IS 'Places a user saved independently of a trip.';
