#include "services/DashboardService.h"

#include "utils/Http.h"

#include <pqxx/pqxx>

DashboardService::DashboardService(Database& database) : database_(database) {}

crow::response DashboardService::stats(const UserRecord& user)
{
    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);

        auto totals = tx.exec_params(
            "SELECT COUNT(*)::int, "
            "COUNT(*) FILTER (WHERE start_date IS NOT NULL AND start_date >= CURRENT_DATE)::int, "
            "COALESCE(SUM(budget),0)::text "
            "FROM trips WHERE user_id = $1",
            user.id);
        auto itineraries = tx.exec_params(
            "SELECT COUNT(DISTINCT t.id)::int FROM trips t "
            "JOIN itinerary_days d ON d.trip_id = t.id WHERE t.user_id = $1",
            user.id);
        auto spending = tx.exec_params(
            "SELECT COALESCE(SUM(amount),0)::text FROM expenses WHERE user_id = $1", user.id);
        auto stories = tx.exec_params("SELECT COUNT(*)::int FROM blogs WHERE user_id = $1", user.id);
        auto recent = tx.exec_params(
            "SELECT id, title, destination, budget::text, duration_days, start_date, created_at, status "
            "FROM trips WHERE user_id = $1 ORDER BY created_at DESC LIMIT 5",
            user.id);
        auto upcoming = tx.exec_params(
            "SELECT id, title, destination, start_date, end_date, duration_days, budget::text "
            "FROM trips WHERE user_id = $1 AND start_date IS NOT NULL AND start_date >= CURRENT_DATE "
            "ORDER BY start_date ASC LIMIT 5",
            user.id);
        auto recentExpenses = tx.exec_params(
            "SELECT e.id, e.category, e.description, e.amount::text, e.expense_date, t.title "
            "FROM expenses e JOIN trips t ON t.id = e.trip_id "
            "WHERE e.user_id = $1 ORDER BY e.created_at DESC LIMIT 5",
            user.id);

        http::Json recentTrips = http::Json::array();
        for (const auto& row : recent)
        {
            recentTrips.push_back({
                {"id", row[0].as<std::int64_t>()},
                {"tripName", row[1].as<std::string>()},
                {"destination", row[2].as<std::string>()},
                {"budget", std::stod(row[3].as<std::string>("0"))},
                {"durationDays", row[4].as<int>()},
                {"startDate", row[5].is_null() ? "" : row[5].as<std::string>()},
                {"createdAt", row[6].as<std::string>()},
                {"status", row[7].as<std::string>()},
            });
        }
        http::Json upcomingTrips = http::Json::array();
        for (const auto& row : upcoming)
        {
            upcomingTrips.push_back({
                {"id", row[0].as<std::int64_t>()},
                {"tripName", row[1].as<std::string>()},
                {"destination", row[2].as<std::string>()},
                {"startDate", row[3].is_null() ? "" : row[3].as<std::string>()},
                {"endDate", row[4].is_null() ? "" : row[4].as<std::string>()},
                {"durationDays", row[5].as<int>()},
                {"budget", std::stod(row[6].as<std::string>("0"))},
            });
        }
        http::Json expenses = http::Json::array();
        for (const auto& row : recentExpenses)
        {
            expenses.push_back({
                {"id", row[0].as<std::int64_t>()},
                {"category", row[1].as<std::string>()},
                {"description", row[2].is_null() ? "" : row[2].as<std::string>()},
                {"amount", std::stod(row[3].as<std::string>("0"))},
                {"expenseDate", row[4].as<std::string>()},
                {"tripName", row[5].as<std::string>()},
            });
        }

        const auto totalBudget = std::stod(totals[0][2].as<std::string>("0"));
        const auto totalSpent = std::stod(spending[0][0].as<std::string>("0"));
        return http::json(200, {
            {"user", userJson(user)},
            {"totalTrips", totals[0][0].as<int>()},
            {"upcomingTripsCount", totals[0][1].as<int>()},
            {"savedItineraries", itineraries[0][0].as<int>()},
            {"totalBudget", totalBudget},
            {"totalSpent", totalSpent},
            {"remainingBudget", totalBudget - totalSpent},
            {"totalStories", stories[0][0].as<int>()},
            {"recentTrips", recentTrips},
            {"upcomingTrips", upcomingTrips},
            {"recentExpenses", expenses},
        });
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not load dashboard stats");
    }
}
