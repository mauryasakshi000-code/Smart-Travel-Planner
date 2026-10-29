#include "controllers/HealthController.h"

#include "database/Database.h"
#include "utils/Http.h"

HealthController::HealthController(Database& database) : database_(database) {}

crow::response HealthController::check() const
{
    if (database_.isHealthy())
    {
        return http::json(200, {{"status", "ok"}, {"database", "connected"}});
    }
    return http::json(503, {{"status", "error"}, {"database", "disconnected"}});
}
