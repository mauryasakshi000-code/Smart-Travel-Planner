CREATE TABLE blog_media (
    id             BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    blog_id        BIGINT       NOT NULL REFERENCES blogs (id) ON DELETE CASCADE,
    cloudinary_url TEXT         NOT NULL,
    public_id      VARCHAR(255) NOT NULL,
    resource_type  VARCHAR(10)  NOT NULL,
    created_at     TIMESTAMPTZ  NOT NULL DEFAULT now(),
    CONSTRAINT blog_media_blog_public_id_key UNIQUE (blog_id, public_id),
    CONSTRAINT blog_media_url_is_cloudinary CHECK (cloudinary_url LIKE 'https://res.cloudinary.com/%'),
    CONSTRAINT blog_media_public_id_not_blank CHECK (btrim(public_id) <> ''),
    CONSTRAINT blog_media_resource_type_allowed CHECK (resource_type IN ('image', 'video'))
);

COMMENT ON TABLE blog_media IS 'Cloudinary media attached to a blog, usually picked from the trip''s media.';
