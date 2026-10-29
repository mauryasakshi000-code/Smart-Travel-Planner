CREATE TABLE blogs (
    id           BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    user_id      BIGINT       NOT NULL REFERENCES users (id) ON DELETE CASCADE,
    trip_id      BIGINT,
    title        VARCHAR(200) NOT NULL,
    content      TEXT         NOT NULL,
    published    BOOLEAN      NOT NULL DEFAULT false,
    published_at TIMESTAMPTZ,
    created_at   TIMESTAMPTZ  NOT NULL DEFAULT now(),
    updated_at   TIMESTAMPTZ  NOT NULL DEFAULT now(),
    -- A blog can only be written from the author's own trip. Deleting the trip
    -- keeps the blog and clears trip_id only.
    CONSTRAINT blogs_trip_owner_fkey FOREIGN KEY (trip_id, user_id)
        REFERENCES trips (id, user_id) ON DELETE SET NULL (trip_id),
    CONSTRAINT blogs_title_not_blank CHECK (btrim(title) <> ''),
    CONSTRAINT blogs_content_not_blank CHECK (btrim(content) <> ''),
    CONSTRAINT blogs_published_at_matches CHECK (published = (published_at IS NOT NULL))
);

COMMENT ON TABLE blogs IS 'Travel blogs. Drafts have published = false; the public feed shows published blogs only.';

CREATE INDEX blogs_user_id_idx ON blogs (user_id);
CREATE INDEX blogs_trip_id_idx ON blogs (trip_id);
-- Public feed: newest published blogs first.
CREATE INDEX blogs_feed_idx ON blogs (published_at DESC) WHERE published;

CREATE TRIGGER blogs_set_updated_at
    BEFORE UPDATE ON blogs
    FOR EACH ROW EXECUTE FUNCTION set_updated_at();
