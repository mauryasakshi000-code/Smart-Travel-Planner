#pragma once

#include "database/Database.h"
#include "models/Types.h"
#include "services/ItineraryPlanner.h"

#include <crow.h>
#include <pqxx/pqxx>

class TripService
{
public:
    explicit TripService(Database& database);

    crow::response create(std::int64_t userId, const crow::request& req);
    crow::response list(std::int64_t userId);
    crow::response get(std::int64_t userId, const std::string& tripId);
    crow::response update(std::int64_t userId, const std::string& tripId, const crow::request& req);
    crow::response remove(std::int64_t userId, const std::string& tripId);
    crow::response generate(std::int64_t userId, const std::string& tripId);
    crow::response itinerary(std::int64_t userId, const std::string& tripId);
    crow::response replaceItinerary(std::int64_t userId, const std::string& tripId, const crow::request& req);
    crow::response activities() const;

private:
    Database& database_;
    ItineraryPlanner planner_;

    http::Json tripRowToJson(const pqxx::row& row, const std::vector<std::string>& activities) const;
};
