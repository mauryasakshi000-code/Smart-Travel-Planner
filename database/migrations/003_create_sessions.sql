CREATE TABLE sessions (
    id            BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    user_id       BIGINT       NOT NULL REFERENCES users (id) ON DELETE CASCADE,
    session_token VARCHAR(128) NOT NULL,
    expires_at    TIMESTAMPTZ  NOT NULL,
    created_at    TIMESTAMPTZ  NOT NULL DEFAULT now(),
    CONSTRAINT sessions_session_token_key UNIQUE (session_token),
    CONSTRAINT sessions_token_min_length CHECK (length(session_token) >= 32),
    CONSTRAINT sessions_expires_after_created CHECK (expires_at > created_at)
);

COMMENT ON TABLE sessions IS 'Login sessions. Deleting a user removes their sessions.';

CREATE INDEX sessions_user_id_idx ON sessions (user_id);
-- Supports periodic cleanup of expired sessions.
CREATE INDEX sessions_expires_at_idx ON sessions (expires_at);
