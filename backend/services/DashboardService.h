#pragma once

#include "database/Database.h"
#include "models/Types.h"

#include <crow.h>

class DashboardService
{
public:
    explicit DashboardService(Database& database);
    crow::response stats(const UserRecord& user);

private:
    Database& database_;
};
