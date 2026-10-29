# syntax=docker/dockerfile:1

# ---------- Build stage ----------
FROM ubuntu:24.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential cmake pkg-config ca-certificates \
      libpqxx-dev libpq-dev libasio-dev nlohmann-json3-dev \
      libargon2-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# The cache mount keeps CMake's build tree (including the Crow download)
# between image builds, so only changed sources are recompiled.
RUN --mount=type=cache,target=/src/build \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --parallel "$(nproc)" \
 && install -m 0755 build/smart_travel_backend /usr/local/bin/smart_travel_backend

# ---------- Runtime stage ----------
FROM ubuntu:24.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends libpqxx-7.8t64 libargon2-1 ca-certificates curl \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --system --no-create-home --shell /usr/sbin/nologin appuser

COPY --from=build /usr/local/bin/smart_travel_backend /usr/local/bin/smart_travel_backend

USER appuser
EXPOSE 8080
CMD ["smart_travel_backend"]