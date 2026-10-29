#!/bin/sh
# Registers the Docker PostgreSQL server in pgAdmin from the same .env values
# used by the postgres service. The password is intentionally not written:
# pgAdmin prompts for it on first connect and can save it in its own volume.
set -e

cat > "${PGADMIN_SERVER_JSON_FILE:-/tmp/servers.json}" <<EOF
{
  "Servers": {
    "1": {
      "Name": "Smart Travel Planner (Docker)",
      "Group": "Servers",
      "Host": "${PGADMIN_TARGET_HOST:-postgres}",
      "Port": ${PGADMIN_TARGET_PORT:-5432},
      "MaintenanceDB": "${POSTGRES_DB}",
      "Username": "${POSTGRES_USER}",
      "ConnectionParameters": {
        "sslmode": "prefer",
        "connect_timeout": 10
      }
    }
  }
}
EOF

exec /entrypoint.sh "$@"
