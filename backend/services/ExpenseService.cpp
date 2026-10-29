#include "services/ExpenseService.h"

#include "utils/Http.h"
#include "utils/JsonBody.h"

#include <pqxx/pqxx>

namespace
{
std::string normalizeCategory(std::string value)
{
    value = jsonBody::lower(jsonBody::trim(value));
    if (value == "misc" || value == "miscellaneous") return "miscellaneous";
    if (value == "stay" || value == "hotel") return "accommodation";
    return value;
}

bool validCategory(const std::string& value)
{
    return value == "food" || value == "transport" || value == "accommodation" || value == "activities"
        || value == "shopping" || value == "other" || value == "miscellaneous";
}

http::Json expenseJson(const pqxx::row& row)
{
    return {
        {"id", row[0].as<std::int64_t>()},
        {"tripId", row[1].as<std::int64_t>()},
        {"category", row[2].as<std::string>()},
        {"description", row[3].is_null() ? "" : row[3].as<std::string>()},
        {"amount", std::stod(row[4].as<std::string>("0"))},
        {"expenseDate", row[5].as<std::string>()},
        {"createdAt", row[6].as<std::string>()},
    };
}

http::Json summaryJson(pqxx::nontransaction& tx, std::int64_t tripId, std::int64_t userId)
{
    auto trip = tx.exec_params(
        "SELECT budget::text, currency FROM trips WHERE id = $1 AND user_id = $2", tripId, userId);
    auto spent = tx.exec_params(
        "SELECT COALESCE(SUM(amount),0)::text FROM expenses WHERE trip_id = $1 AND user_id = $2", tripId, userId);
    auto estimated = tx.exec_params(
        "SELECT COALESCE(SUM(l.estimated_cost),0)::text FROM itinerary_locations l "
        "JOIN itinerary_days d ON d.id = l.itinerary_day_id WHERE d.trip_id = $1",
        tripId);
    auto byCat = tx.exec_params(
        "SELECT category, COALESCE(SUM(amount),0)::text FROM expenses WHERE trip_id = $1 AND user_id = $2 "
        "GROUP BY category",
        tripId, userId);
    const auto budget = std::stod(trip[0][0].as<std::string>("0"));
    const auto totalSpent = std::stod(spent[0][0].as<std::string>("0"));
    http::Json breakdown = http::Json::object();
    for (const auto& row : byCat)
    {
        breakdown[row[0].as<std::string>()] = std::stod(row[1].as<std::string>("0"));
    }
    return {
        {"totalBudget", budget},
        {"currency", trip[0][1].as<std::string>()},
        {"totalSpent", totalSpent},
        {"remainingBudget", budget - totalSpent},
        {"estimatedItineraryCost", std::stod(estimated[0][0].as<std::string>("0"))},
        {"categoryBreakdown", breakdown},
        {"estimated", true},
    };
}
} // namespace

ExpenseService::ExpenseService(Database& database) : database_(database) {}

crow::response ExpenseService::add(std::int64_t userId, const std::string& tripId, const crow::request& req)
{
    const auto id = jsonBody::asId(tripId);
    if (!id) return http::error(400, "Invalid trip id");
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok) return http::error(parsed.status, parsed.error);

    const auto category = normalizeCategory(parsed.json.value("category", ""));
    const auto description = jsonBody::trim(parsed.json.value("description", ""));
    const auto amount = parsed.json.value("amount", 0.0);
    const auto date = jsonBody::trim(parsed.json.value("expenseDate", parsed.json.value("date", "")));
    if (!validCategory(category)) return http::error(400, "Invalid expense category");
    if (amount <= 0) return http::error(400, "Expense amount must be greater than zero");

    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto trip = tx.exec_params("SELECT id FROM trips WHERE id = $1 AND user_id = $2", *id, userId);
        if (trip.empty()) return http::error(404, "Trip not found");
        auto inserted = tx.exec_params(
            "INSERT INTO expenses (trip_id, user_id, category, description, amount, expense_date) "
            "VALUES ($1, $2, $3, NULLIF($4,''), $5, COALESCE(NULLIF($6,'')::date, CURRENT_DATE)) "
            "RETURNING id, trip_id, category, description, amount::text, expense_date, created_at",
            *id, userId, category, description, amount, date);
        tx.commit();
        pqxx::nontransaction read(*connection);
        auto body = expenseJson(inserted[0]);
        return http::json(201, {{"expense", body}, {"summary", summaryJson(read, *id, userId)}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not add the expense");
    }
}

crow::response ExpenseService::list(std::int64_t userId, const std::string& tripId)
{
    const auto id = jsonBody::asId(tripId);
    if (!id) return http::error(400, "Invalid trip id");
    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);
        auto trip = tx.exec_params("SELECT id FROM trips WHERE id = $1 AND user_id = $2", *id, userId);
        if (trip.empty()) return http::error(404, "Trip not found");
        auto rows = tx.exec_params(
            "SELECT id, trip_id, category, description, amount::text, expense_date, created_at "
            "FROM expenses WHERE trip_id = $1 AND user_id = $2 ORDER BY expense_date DESC, id DESC",
            *id, userId);
        http::Json list = http::Json::array();
        for (const auto& row : rows) list.push_back(expenseJson(row));
        return http::json(200, {{"expenses", list}, {"summary", summaryJson(tx, *id, userId)}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not load expenses");
    }
}

crow::response ExpenseService::update(std::int64_t userId, const std::string& expenseId, const crow::request& req)
{
    const auto id = jsonBody::asId(expenseId);
    if (!id) return http::error(400, "Invalid expense id");
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok) return http::error(parsed.status, parsed.error);
    const auto category = normalizeCategory(parsed.json.value("category", ""));
    const auto description = jsonBody::trim(parsed.json.value("description", ""));
    const auto amount = parsed.json.value("amount", 0.0);
    const auto date = jsonBody::trim(parsed.json.value("expenseDate", parsed.json.value("date", "")));
    if (!validCategory(category)) return http::error(400, "Invalid expense category");
    if (amount <= 0) return http::error(400, "Expense amount must be greater than zero");

    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto updated = tx.exec_params(
            "UPDATE expenses SET category=$1, description=NULLIF($2,''), amount=$3, "
            "expense_date=COALESCE(NULLIF($4,'')::date, expense_date) "
            "WHERE id=$5 AND user_id=$6 "
            "RETURNING id, trip_id, category, description, amount::text, expense_date, created_at",
            category, description, amount, date, *id, userId);
        if (updated.empty()) return http::error(404, "Expense not found");
        const auto tripId = updated[0][1].as<std::int64_t>();
        tx.commit();
        pqxx::nontransaction read(*connection);
        return http::json(200, {{"expense", expenseJson(updated[0])}, {"summary", summaryJson(read, tripId, userId)}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not update the expense");
    }
}

crow::response ExpenseService::remove(std::int64_t userId, const std::string& expenseId)
{
    const auto id = jsonBody::asId(expenseId);
    if (!id) return http::error(400, "Invalid expense id");
    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto deleted = tx.exec_params(
            "DELETE FROM expenses WHERE id = $1 AND user_id = $2 RETURNING trip_id", *id, userId);
        if (deleted.empty()) return http::error(404, "Expense not found");
        const auto tripId = deleted[0][0].as<std::int64_t>();
        tx.commit();
        pqxx::nontransaction read(*connection);
        return http::json(200, {{"ok", true}, {"summary", summaryJson(read, tripId, userId)}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not delete the expense");
    }
}
