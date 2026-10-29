#include "routes/Routes.h"

#include "utils/Http.h"

namespace routes
{
void registerFallbackRoute(CrowApp& app)
{
    CROW_CATCHALL_ROUTE(app)([]() {
        return http::error(404, "Route not found");
    });
}
} // namespace routes
