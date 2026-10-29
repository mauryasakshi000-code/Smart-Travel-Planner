#pragma once

#include "App.h"

class HealthController;
class AuthService;
class TripService;
class ExpenseService;
class DashboardService;
class ContentService;

namespace routes
{
void registerHealthRoutes(CrowApp& app, HealthController& controller);

void registerApiRoutes(CrowApp& app, AuthService& auth, TripService& trips, ExpenseService& expenses,
                       DashboardService& dashboard, ContentService& content);

// Must be registered last: returns a JSON 404 for any unmatched path.
void registerFallbackRoute(CrowApp& app);
} // namespace routes