CREATE TABLE trips (
    id            BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    user_id       BIGINT        NOT NULL REFERENCES users (id) ON DELETE CASCADE,
    title         VARCHAR(150)  NOT NULL,
    destination   VARCHAR(150)  NOT NULL,
    budget        NUMERIC(12,2) NOT NULL,
    duration_days INTEGER       NOT NULL,
    interests     TEXT[]        NOT NULL DEFAULT '{}',
    created_at    TIMESTAMPTZ   NOT NULL DEFAULT now(),
    updated_at    TIMESTAMPTZ   NOT NULL DEFAULT now(),
    -- Lets blogs and expenses reference (trip, owner) as a pair, so rows can
    -- only be attached to a trip owned by the same user.
    CONSTRAINT trips_id_user_id_key UNIQUE (id, user_id),
    CONSTRAINT trips_title_not_blank CHECK (btrim(title) <> ''),
    CONSTRAINT trips_destination_not_blank CHECK (btrim(destination) <> ''),
    CONSTRAINT trips_budget_non_negative CHECK (budget >= 0),
    CONSTRAINT trips_duration_range CHECK (duration_days BETWEEN 1 AND 60),
    CONSTRAINT trips_interests_allowed CHECK (
        interests <@ ARRAY['adventure', 'food', 'culture', 'history',
                           'nature', 'shopping', 'nightlife', 'photography']::TEXT[]
    )
);

COMMENT ON TABLE trips IS 'Planned trips. Budget is in rupees (INR).';

CREATE INDEX trips_user_id_created_at_idx ON trips (user_id, created_at DESC);
-- Supports interest filters such as: WHERE interests @> ARRAY['food'].
CREATE INDEX trips_interests_idx ON trips USING GIN (interests);

CREATE TRIGGER trips_set_updated_at
    BEFORE UPDATE ON trips
    FOR EACH ROW EXECUTE FUNCTION set_updated_at();
