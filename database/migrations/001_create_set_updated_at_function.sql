-- Trigger function that keeps updated_at current on every UPDATE.
-- Attached to users, trips, blogs and blog_comments.
CREATE FUNCTION set_updated_at() RETURNS trigger
LANGUAGE plpgsql AS $$
BEGIN
    NEW.updated_at := now();
    RETURN NEW;
END;
$$;
