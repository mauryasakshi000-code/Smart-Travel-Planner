#include "routes/Routes.h"

#include "controllers/HealthController.h"

namespace routes
{
void registerHealthRoutes(CrowApp& app, HealthController& controller)
{
    CROW_ROUTE(app, "/health").methods(crow::HTTPMethod::Get)([&controller]() {
        return controller.check();
    });
}
} // namespace routes
