CREATE TABLE trip_media (
    id             BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    trip_id        BIGINT       NOT NULL REFERENCES trips (id) ON DELETE CASCADE,
    cloudinary_url TEXT         NOT NULL,
    public_id      VARCHAR(255) NOT NULL,
    resource_type  VARCHAR(10)  NOT NULL,
    created_at     TIMESTAMPTZ  NOT NULL DEFAULT now(),
    CONSTRAINT trip_media_trip_public_id_key UNIQUE (trip_id, public_id),
    CONSTRAINT trip_media_url_is_cloudinary CHECK (cloudinary_url LIKE 'https://res.cloudinary.com/%'),
    CONSTRAINT trip_media_public_id_not_blank CHECK (btrim(public_id) <> ''),
    CONSTRAINT trip_media_resource_type_allowed CHECK (resource_type IN ('image', 'video'))
);

COMMENT ON TABLE trip_media IS 'Cloudinary metadata for trip photos and videos. The files themselves live in Cloudinary.';
