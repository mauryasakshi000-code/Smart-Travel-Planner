CREATE TABLE blog_likes (
    id         BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    blog_id    BIGINT      NOT NULL REFERENCES blogs (id) ON DELETE CASCADE,
    user_id    BIGINT      NOT NULL REFERENCES users (id) ON DELETE CASCADE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    -- A user can like a given blog only once.
    CONSTRAINT blog_likes_blog_user_key UNIQUE (blog_id, user_id)
);

COMMENT ON TABLE blog_likes IS 'One row per (blog, user) like.';

CREATE INDEX blog_likes_user_id_idx ON blog_likes (user_id);
