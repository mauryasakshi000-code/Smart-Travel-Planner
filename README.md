# Smart Travel Planner & Blogging Web System

A full-stack travel planner: React frontend, C++ (Crow) REST backend, PostgreSQL in Docker, pgAdmin for database management, and Cloudinary for media.

> **Current status: Phase 2 — PostgreSQL database schema, migrations and seed data.**
> Authentication, trips, blogs and expenses APIs are built in later phases (see [Roadmap](#roadmap)).

## Architecture

```
React frontend (Vite, :5173)
      │  /api/* is proxied to the backend
      ▼
C++ REST backend (Crow + libpqxx, :8080)
      │  connection pool, parameterised SQL
      ▼
PostgreSQL 17 (Docker, :5432) ◄──── pgAdmin 4 (Docker, :5050)
      │
      └─ named volume "postgres_data" (persistent)

All four containers share the Docker bridge network "travel_network".
Cloudinary (later phases) is external: the browser uploads to it directly.
```

## Prerequisites

- **Docker Desktop** (includes Docker Compose v2). Nothing else is required: the C++ backend and the React frontend are both built inside containers.
- Optional, for running outside Docker: Node.js 20.19+ / 22.12+, CMake 3.25+, a C++17 compiler, libpqxx 7, standalone Asio and nlohmann-json (see [Running without Docker](#running-without-docker)).

## Quick start

```powershell
# 1. Create your local environment file and change the passwords in it
Copy-Item .env.example .env        # macOS/Linux: cp .env.example .env

# 2. Build and start everything
docker compose up -d --build

# 3. Check that all services are running (postgres and backend should become "healthy").
#    The one-shot "migrate" job creates the tables first; see it with: docker compose ps -a
docker compose ps

# 4. Check the backend health endpoint
curl http://localhost:8080/health
```

The first build takes several minutes (it installs a C++ toolchain and downloads Crow). Later builds reuse the cache.

## Service reference

| What | Value |
| --- | --- |
| Frontend URL | http://localhost:5173 |
| Backend URL | http://localhost:8080 (health: http://localhost:8080/health) |
| Backend via frontend proxy | http://localhost:5173/api/health |
| pgAdmin URL | http://localhost:5050 |
| pgAdmin login | `PGADMIN_DEFAULT_EMAIL` / `PGADMIN_DEFAULT_PASSWORD` from `.env` |
| PostgreSQL host (from your machine) | `localhost`, port `5432` (`POSTGRES_HOST_PORT`) |
| PostgreSQL host (from containers, incl. pgAdmin) | `postgres`, port `5432` |
| Database name | `smart_travel_planner` (`POSTGRES_DB`) |
| Database user / password | `POSTGRES_USER` / `POSTGRES_PASSWORD` from `.env` |

### Connecting pgAdmin to PostgreSQL

The server **"Smart Travel Planner (Docker)"** is registered automatically from your `.env` values (see `docker/pgadmin/entrypoint.sh`).

1. Open http://localhost:5050 and log in with the pgAdmin email and password.
2. Expand **Servers → Smart Travel Planner (Docker)**.
3. Enter `POSTGRES_PASSWORD` when prompted (tick *Save password* to skip this next time).

To add it manually instead: host `postgres` (not `localhost` — pgAdmin runs in its own container), port `5432`, maintenance database `smart_travel_planner`, username `POSTGRES_USER`.

## Database schema and migrations

Migrations live in `database/migrations/`, one numbered SQL file per step. The `migrate` service (`docker/migrate/migrate.sh`) runs automatically on every `docker compose up`, before the backend starts:

- Each file runs once, inside its own transaction, and is recorded in the `schema_migrations` table. Files already applied are skipped.
- When `RUN_SEEDS=true` (the default for development), `database/seeds/*.sql` are applied the same way afterwards.
- To add a change, create the next numbered file (for example `014_add_trip_start_date.sql`). Never edit a migration that has already been applied.

```powershell
docker compose run --rm migrate      # apply any new migrations/seeds now
docker compose ps -a                 # "migrate" should show Exited (0)
```

### Tables

| Table | Purpose | Deleted automatically when… |
| --- | --- | --- |
| `users` | Accounts (email unique and stored lower-case, password hash only) | — |
| `sessions` | Login sessions with expiry | its user is deleted |
| `trips` | Destination, budget (INR), duration, interests (`TEXT[]`) | its user is deleted |
| `itinerary_days` | Day-wise plan, unique per (trip, day_number) | its trip is deleted |
| `itinerary_locations` | Recommended places with category, estimated cost, coordinates | its day is deleted |
| `trip_media` | Cloudinary URL, `public_id`, `resource_type` only | its trip is deleted |
| `blogs` | Blog written from a trip; `published` + `published_at` | its author is deleted (deleting the trip only clears `trip_id`) |
| `blog_tags` | One row per tag | its blog is deleted |
| `blog_media` | Cloudinary media attached to a blog | its blog is deleted |
| `blog_likes` | `UNIQUE (blog_id, user_id)`: one like per user per blog | its blog or user is deleted |
| `blog_comments` | Comments | its blog or user is deleted |
| `expenses` | Categorised trip expenses (INR) | its trip or user is deleted |

Rules enforced by the database itself:

- Money columns are `NUMERIC(12,2)`. Budgets and estimated costs cannot be negative; expense amounts must be greater than zero.
- Trip interests must be from: adventure, food, culture, history, nature, shopping, nightlife, photography.
- Expense categories must be from: food, transport, accommodation, activities, shopping, other.
- Media URLs must start with `https://res.cloudinary.com/` and `resource_type` must be `image` or `video`. No file data is stored.
- Blogs and expenses can only be attached to a trip owned by the same user (composite foreign key on `(trip_id, user_id)`).
- `updated_at` is maintained by a trigger on `users`, `trips`, `blogs` and `blog_comments`.

### Seed data

`database/seeds/001_dev_seed.sql` loads 5 users, 6 trips (Paris, Goa, Tokyo, Jaipur, Manali, Kerala) with 21 itinerary days and 42 locations, 14 media items, 6 blogs (4 published, 2 drafts), 25 tags, 11 likes, 8 comments, 43 expenses and 3 expired sessions. A few gaps are deliberate for practising joins: Kabir Singh has no trips, and the Goa trip is over budget.

Seed accounts use a placeholder password hash and cannot log in. The media URLs point at Cloudinary's public `demo` samples.

### Example queries

Run these in pgAdmin (**Tools → Query Tool**) or in `psql`:

```sql
-- Budget, amount spent and remaining budget per trip
SELECT t.title, t.budget,
       COALESCE(SUM(e.amount), 0)            AS spent,
       t.budget - COALESCE(SUM(e.amount), 0) AS remaining
FROM trips t
LEFT JOIN expenses e ON e.trip_id = t.id
GROUP BY t.id
ORDER BY remaining;

-- Public feed: published blogs with author, destination, likes and comments
SELECT b.title, u.full_name AS author, t.destination, b.published_at::date AS published,
       (SELECT count(*) FROM blog_likes    l WHERE l.blog_id = b.id) AS likes,
       (SELECT count(*) FROM blog_comments c WHERE c.blog_id = b.id) AS comments
FROM blogs b
JOIN users u      ON u.id = b.user_id
LEFT JOIN trips t ON t.id = b.trip_id
WHERE b.published
ORDER BY b.published_at DESC;

-- Day-by-day itinerary for one trip
SELECT d.day_number, d.title AS day, l.name, l.category, l.estimated_cost
FROM trips t
JOIN itinerary_days d      ON d.trip_id = t.id
JOIN itinerary_locations l ON l.itinerary_day_id = d.id
WHERE t.title = 'Tokyo Cherry Blossom Trip'
ORDER BY d.day_number, l.id;

-- Trips that include the "food" interest
SELECT title, interests FROM trips WHERE interests @> ARRAY['food'];

-- Users who have not planned any trip
SELECT u.full_name
FROM users u
LEFT JOIN trips t ON t.user_id = u.id
WHERE t.id IS NULL;
```

### Resetting the database to the seed data

This deletes **all** data in the database and rebuilds it from the migrations and seeds:

```powershell
docker compose exec postgres psql -U travel_admin -d smart_travel_planner -c "DROP SCHEMA public CASCADE; CREATE SCHEMA public;"
docker compose run --rm migrate
```

## Health endpoint

`GET /health` runs `SELECT 1` through the backend's connection pool.

```json
// 200 OK
{ "status": "ok", "database": "connected" }

// 503 Service Unavailable (PostgreSQL unreachable)
{ "status": "error", "database": "disconnected" }
```

Unknown routes return `404 {"error": "Route not found"}`.

## Environment variables

All configuration comes from `.env` (git-ignored). `.env.example` documents every variable.

| Variable | Used by | Purpose |
| --- | --- | --- |
| `POSTGRES_USER`, `POSTGRES_PASSWORD`, `POSTGRES_DB` | postgres, backend, pgadmin | Database credentials and name |
| `POSTGRES_HOST_PORT` | postgres | Host port for PostgreSQL (default 5432) |
| `RUN_SEEDS` | migrate | `true` loads the development seed data after migrations (default `true`) |
| `PGADMIN_DEFAULT_EMAIL`, `PGADMIN_DEFAULT_PASSWORD` | pgadmin | pgAdmin web login |
| `PGADMIN_HOST_PORT` | pgadmin | Host port for pgAdmin (default 5050) |
| `BACKEND_HOST_PORT` | backend | Host port for the API (default 8080) |
| `SERVER_THREADS` | backend | Crow worker threads |
| `DB_POOL_SIZE` | backend | Maximum PostgreSQL connections held by the backend |
| `CORS_ORIGIN` | backend | Browser origin allowed to call the API directly |
| `FRONTEND_HOST_PORT` | frontend | Host port for the Vite dev server (keep 5173) |

Inside the backend container Compose maps these to `DB_HOST`, `DB_PORT`, `DB_NAME`, `DB_USER`, `DB_PASSWORD` and `SERVER_PORT`.

## Project structure

```
smart-travel-planner/
├── backend/                 C++17 Crow REST API
│   ├── include/             App-wide headers (App type, config, server)
│   ├── src/                 App bootstrap (config loading, server wiring)
│   ├── routes/              Route registration (URL → controller)
│   ├── controllers/         Request handling (HealthController)
│   ├── models/              Domain models (Phase 3+)
│   ├── services/            Business logic (Phase 3+)
│   ├── database/            PostgreSQL connection pool (libpqxx)
│   ├── middleware/          Auth middleware (Phase 3+)
│   ├── utils/               Env parsing, JSON response helpers
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   └── main.cpp
├── frontend/                React 19 + Vite + React Router
│   └── src/
│       ├── components/      Reusable UI (Navbar, StatusRow)
│       ├── pages/           Route pages (HomePage, NotFoundPage)
│       ├── services/        API client (fetch wrapper with timeouts)
│       ├── context/         Global state (auth, Phase 3+)
│       ├── hooks/           Custom hooks (useHealthCheck)
│       └── utils/           Formatting helpers
├── database/
│   ├── migrations/          Numbered SQL schema migrations (001_…sql to 013_…sql)
│   └── seeds/               Development seed data
├── docker/
│   ├── migrate/             Migration runner used by the "migrate" service
│   └── pgadmin/             pgAdmin startup script (server registration)
├── docker-compose.yml
├── .env.example
└── README.md
```

## Useful commands

```powershell
docker compose logs -f backend        # follow backend logs
docker compose restart backend        # restart one service
docker compose up -d --build backend  # rebuild after C++ changes
docker compose down                   # stop everything (data is kept)
docker compose down -v                # stop AND DELETE the database volume

# psql shell inside the database container
docker compose exec postgres psql -U travel_admin -d smart_travel_planner
```

Frontend source is bind-mounted, so React changes hot-reload without a rebuild. After changing `frontend/package.json`, run `docker compose up -d --build -V frontend` so the container gets fresh `node_modules`.

## Running without Docker

Keep PostgreSQL in Docker (`docker compose up -d postgres pgadmin`) and run the app natively:

**Backend** (Linux/WSL, Ubuntu 24.04 packages shown):

```bash
sudo apt install build-essential cmake pkg-config libpqxx-dev libasio-dev nlohmann-json3-dev
cd backend
cmake -S . -B build && cmake --build build -j
DB_HOST=localhost DB_NAME=smart_travel_planner DB_USER=travel_admin DB_PASSWORD=... ./build/smart_travel_backend
```

On Windows, install the same libraries with vcpkg (`libpqxx`, `asio`, `nlohmann-json`) and pass `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake`.

**Frontend:**

```bash
cd frontend
npm install
npm run dev     # proxies /api to http://localhost:8080
```

## Troubleshooting

- **Port already in use** — change `POSTGRES_HOST_PORT`, `PGADMIN_HOST_PORT` or `BACKEND_HOST_PORT` in `.env`.
- **pgAdmin or migrate exits with `illegal option` / `not found` on Windows** — the `.sh` files in `docker/` must use LF line endings (enforced by `.gitattributes`). Re-save them with LF endings.
- **`migrate` exits with an error** — run `docker compose logs migrate`. The failing file is rolled back and not recorded, so fix it and run `docker compose run --rm migrate` again.
- **`/health` returns 503** — check `docker compose ps` for the postgres container and `docker compose logs backend` for connection errors.
- **Changed `POSTGRES_PASSWORD` but login fails** — PostgreSQL only reads it when the volume is first created. Either change it with `ALTER USER` or reset with `docker compose down -v` (deletes data).

## Roadmap

1. ✅ Project setup, Docker & PostgreSQL
2. ✅ Database schema & migrations
3. C++ models & authentication
4. Smart trip planner
5. Trip CRUD
6. Cloudinary media
7. Blogging system
8. Likes & comments
9. Expense tracker
10. Dashboard
11. Security & error handling audit
12. Full Dockerisation (production images)
13. End-to-end testing
