CREATE TABLE blog_comments (
    id         BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    blog_id    BIGINT      NOT NULL REFERENCES blogs (id) ON DELETE CASCADE,
    user_id    BIGINT      NOT NULL REFERENCES users (id) ON DELETE CASCADE,
    content    TEXT        NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    CONSTRAINT blog_comments_content_length CHECK (btrim(content) <> '' AND length(content) <= 2000)
);

COMMENT ON TABLE blog_comments IS 'Comments on blogs.';

CREATE INDEX blog_comments_blog_id_created_at_idx ON blog_comments (blog_id, created_at);
CREATE INDEX blog_comments_user_id_idx ON blog_comments (user_id);

CREATE TRIGGER blog_comments_set_updated_at
    BEFORE UPDATE ON blog_comments
    FOR EACH ROW EXECUTE FUNCTION set_updated_at();
