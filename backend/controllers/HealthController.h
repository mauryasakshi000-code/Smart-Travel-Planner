#pragma once

#include <crow.h>

class Database;

class HealthController
{
public:
    explicit HealthController(Database& database);

    // 200 {"status":"ok","database":"connected"} or 503 when PostgreSQL is unreachable.
    crow::response check() const;

private:
    Database& database_;
};
