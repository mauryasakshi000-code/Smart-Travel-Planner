#!/bin/sh
# Applies /database/migrations/*.sql, then /database/seeds/*.sql when
# RUN_SEEDS=true, in filename order. Each file runs in its own transaction and
# is recorded in schema_migrations, so later runs skip files already applied.
# Connection settings come from the standard PGHOST/PGUSER/... variables.
set -eu

run_psql() {
    psql -X -q -v ON_ERROR_STOP=1 "$@"
}

run_psql -c "CREATE TABLE IF NOT EXISTS schema_migrations (
    filename   TEXT PRIMARY KEY,
    applied_at TIMESTAMPTZ NOT NULL DEFAULT now()
)"

apply_dir() {
    dir="$1"
    kind="$2"

    for file in "$dir"/*.sql; do
        [ -e "$file" ] || continue

        base=$(basename "$file")
        # The name is embedded in SQL below, so only allow safe characters.
        case "$base" in
            *[!A-Za-z0-9_.-]*)
                echo "Refusing to apply $file: file names may only contain letters, digits, '_', '-' and '.'" >&2
                exit 1
                ;;
        esac
        name="$kind/$base"

        if [ "$(run_psql -tA -c "SELECT 1 FROM schema_migrations WHERE filename = '$name'")" = "1" ]; then
            echo "skip   $name"
            continue
        fi

        echo "apply  $name"
        run_psql --single-transaction -f "$file" \
            -c "INSERT INTO schema_migrations (filename) VALUES ('$name')"
    done
}

apply_dir /database/migrations migrations

if [ "${RUN_SEEDS:-false}" = "true" ]; then
    apply_dir /database/seeds seeds
else
    echo "RUN_SEEDS is not 'true'; skipping seed data."
fi

echo "Database is up to date."
