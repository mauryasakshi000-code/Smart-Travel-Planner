#pragma once

#include "App.h"
#include "AppConfig.h"
#include "controllers/HealthController.h"
#include "database/Database.h"
#include "services/AuthService.h"
#include "services/ContentService.h"
#include "services/DashboardService.h"
#include "services/ExpenseService.h"
#include "services/TripService.h"

class Server
{
public:
    explicit Server(AppConfig config);

    int run();

private:
    void configureCors();
    void registerRoutes();

    AppConfig config_;
    Database database_;
    HealthController healthController_;
    AuthService auth_;
    TripService trips_;
    ExpenseService expenses_;
    DashboardService dashboard_;
    ContentService content_;
    CrowApp app_;
};