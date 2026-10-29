#pragma once

#include "database/Database.h"

#include <crow.h>
#include <cstdint>
#include <string>

class ExpenseService
{
public:
    explicit ExpenseService(Database& database);

    crow::response add(std::int64_t userId, const std::string& tripId, const crow::request& req);
    crow::response list(std::int64_t userId, const std::string& tripId);
    crow::response update(std::int64_t userId, const std::string& expenseId, const crow::request& req);
    crow::response remove(std::int64_t userId, const std::string& expenseId);

private:
    Database& database_;
};
