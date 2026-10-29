CREATE TABLE blog_tags (
    id      BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    blog_id BIGINT      NOT NULL REFERENCES blogs (id) ON DELETE CASCADE,
    tag     VARCHAR(40) NOT NULL,
    CONSTRAINT blog_tags_blog_tag_key UNIQUE (blog_id, tag),
    -- Lowercase letters, digits and hyphens, e.g. "cherry-blossom".
    CONSTRAINT blog_tags_tag_format CHECK (tag ~ '^[a-z0-9][a-z0-9-]*$')
);

COMMENT ON TABLE blog_tags IS 'Tags on a blog, one row per tag.';

CREATE INDEX blog_tags_tag_idx ON blog_tags (tag);
