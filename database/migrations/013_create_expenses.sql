CREATE TABLE expenses (
    id           BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    trip_id      BIGINT        NOT NULL,
    user_id      BIGINT        NOT NULL REFERENCES users (id) ON DELETE CASCADE,
    category     VARCHAR(20)   NOT NULL,
    description  VARCHAR(255),
    amount       NUMERIC(12,2) NOT NULL,
    expense_date DATE          NOT NULL DEFAULT CURRENT_DATE,
    created_at   TIMESTAMPTZ   NOT NULL DEFAULT now(),
    -- Every expense belongs to a trip owned by the same user.
    CONSTRAINT expenses_trip_owner_fkey FOREIGN KEY (trip_id, user_id)
        REFERENCES trips (id, user_id) ON DELETE CASCADE,
    CONSTRAINT expenses_category_allowed CHECK (
        category IN ('food', 'transport', 'accommodation', 'activities', 'shopping', 'other')
    ),
    CONSTRAINT expenses_amount_positive CHECK (amount > 0)
);

COMMENT ON TABLE expenses IS 'Money spent on a trip, in INR. Remaining budget = trips.budget - SUM(amount).';

CREATE INDEX expenses_trip_id_expense_date_idx ON expenses (trip_id, expense_date);
CREATE INDEX expenses_user_id_idx ON expenses (user_id);
