#include "services/TripService.h"

#include "utils/Http.h"
#include "utils/JsonBody.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <crow/logging.h>
#include <ctime>
#include <sstream>

namespace
{
constexpr const char* kTripSelect =
    "SELECT t.id, t.user_id, t.title, t.destination, t.budget::text, t.duration_days, t.interests, "
    "t.created_at, t.updated_at, t.start_date, t.end_date, t.currency, t.members, t.travel_style, "
    "t.accommodation_preference, t.transportation_preference, t.dietary_preferences, "
    "t.special_requirements, t.notes, t.status FROM trips t";

int daysBetween(const std::string& start, const std::string& end)
{
    int y1, m1, d1, y2, m2, d2;
    if (std::sscanf(start.c_str(), "%d-%d-%d", &y1, &m1, &d1) != 3) return 0;
    if (std::sscanf(end.c_str(), "%d-%d-%d", &y2, &m2, &d2) != 3) return 0;
    std::tm a{};
    a.tm_year = y1 - 1900;
    a.tm_mon = m1 - 1;
    a.tm_mday = d1;
    std::tm b{};
    b.tm_year = y2 - 1900;
    b.tm_mon = m2 - 1;
    b.tm_mday = d2;
    const auto ta = std::mktime(&a);
    const auto tb = std::mktime(&b);
    if (ta == -1 || tb == -1) return 0;
    return static_cast<int>(std::difftime(tb, ta) / 86400.0) + 1;
}

std::vector<std::string> parseActivities(const http::Json& body)
{
    std::vector<std::string> out;
    http::Json source = http::Json::array();
    if (body.contains("activities") && body["activities"].is_array()) source = body["activities"];
    else if (body.contains("interests") && body["interests"].is_array()) source = body["interests"];
    else if (body.contains("preferredActivities") && body["preferredActivities"].is_array())
        source = body["preferredActivities"];

    for (const auto& item : source)
    {
        auto value = jsonBody::lower(jsonBody::trim(item.get<std::string>()));
        std::replace(value.begin(), value.end(), ' ', '_');
        if (value == "food_tours") value = "food";
        if (value == "nature_and_hiking") value = "nature";
        if (value == "adventure_sports") value = "adventure";
        if (value == "cultural_experiences") value = "culture";
        if (isAllowedActivity(value) && std::find(out.begin(), out.end(), value) == out.end())
        {
            out.push_back(value);
        }
    }
    return out;
}

std::string pgTextArray(const std::vector<std::string>& values)
{
    std::ostringstream out;
    out << "{";
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i) out << ",";
        out << values[i];
    }
    out << "}";
    return out.str();
}

struct TripFields
{
    std::string title;
    std::string destination;
    std::string startDate;
    std::string endDate;
    int duration = 0;
    double budget = 0;
    std::string currency = "INR";
    int members = 1;
    std::string travelStyle = "standard";
    std::string accommodation;
    std::string transportation;
    std::string dietary;
    std::string special;
    std::string notes;
    std::string status = "planned";
    std::vector<std::string> activities;
    std::string error;
    bool ok = false;
};

TripFields parseTripBody(const http::Json& body, bool partial)
{
    TripFields fields;
    fields.title = jsonBody::trim(body.value("tripName", body.value("title", "")));
    fields.destination = jsonBody::trim(body.value("destination", ""));
    fields.startDate = jsonBody::trim(body.value("startDate", body.value("start_date", "")));
    fields.endDate = jsonBody::trim(body.value("endDate", body.value("end_date", "")));
    fields.currency = jsonBody::trim(body.value("currency", "INR"));
    std::transform(fields.currency.begin(), fields.currency.end(), fields.currency.begin(), ::toupper);
    fields.travelStyle = jsonBody::lower(jsonBody::trim(body.value("travelStyle", body.value("travel_style", "standard"))));
    fields.accommodation = jsonBody::trim(body.value("accommodationPreference", body.value("accommodation_preference", "")));
    fields.transportation = jsonBody::trim(body.value("transportationPreference", body.value("transportation_preference", "")));
    fields.dietary = jsonBody::trim(body.value("dietaryPreferences", body.value("dietary_preferences", "")));
    fields.special = jsonBody::trim(body.value("specialRequirements", body.value("special_requirements", "")));
    fields.notes = jsonBody::trim(body.value("notes", ""));
    fields.status = jsonBody::lower(jsonBody::trim(body.value("status", "planned")));
    fields.activities = parseActivities(body);

    if (body.contains("durationDays") || body.contains("duration") || body.contains("numberOfDays"))
    {
        fields.duration = body.value("durationDays", body.value("duration", body.value("numberOfDays", 0)));
    }
    if (body.contains("budget") || body.contains("totalBudget"))
    {
        fields.budget = body.value("budget", body.value("totalBudget", 0.0));
    }
    fields.members = body.value("members", body.value("numberOfTravelers", 1));

    if (!partial)
    {
        if (fields.title.size() < 2) { fields.error = "Trip name is required"; return fields; }
        if (fields.destination.empty()) { fields.error = "Destination is required"; return fields; }
        if (fields.budget < 0) { fields.error = "Budget cannot be negative"; return fields; }
        if (fields.members < 1) { fields.error = "Number of travelers must be at least 1"; return fields; }
    }

    if (!fields.startDate.empty() && !fields.endDate.empty())
    {
        const int span = daysBetween(fields.startDate, fields.endDate);
        if (span < 1)
        {
            fields.error = "Ending date cannot precede starting date";
            return fields;
        }
        fields.duration = span;
    }
    if (fields.duration < 1)
    {
        if (!partial)
        {
            fields.error = "Provide dates or a duration of at least 1 day";
            return fields;
        }
    }
    else if (fields.duration > 60)
    {
        fields.error = "Duration cannot exceed 60 days";
        return fields;
    }

    if (fields.currency.size() != 3)
    {
        fields.error = "Currency must be a 3-letter code such as INR";
        return fields;
    }
    if (fields.travelStyle != "budget" && fields.travelStyle != "standard" && fields.travelStyle != "luxury"
        && fields.travelStyle != "adventure")
    {
        fields.error = "Travel style must be budget, standard, luxury or adventure";
        return fields;
    }
    if (fields.status != "draft" && fields.status != "planned" && fields.status != "completed"
        && fields.status != "cancelled")
    {
        fields.status = "planned";
    }
    fields.ok = true;
    return fields;
}

void saveActivities(pqxx::work& tx, std::int64_t tripId, const std::vector<std::string>& activities)
{
    tx.exec_params("DELETE FROM trip_activities WHERE trip_id = $1", tripId);
    for (const auto& name : activities)
    {
        tx.exec_params("INSERT INTO trip_activities (trip_id, activity_name) VALUES ($1, $2)", tripId, name);
    }
}

void saveItinerary(pqxx::work& tx, std::int64_t tripId, const PlannerResult& plan)
{
    tx.exec_params("DELETE FROM itinerary_days WHERE trip_id = $1", tripId);
    for (const auto& day : plan.days)
    {
        auto inserted = tx.exec_params(
            "INSERT INTO itinerary_days (trip_id, day_number, date, title, description, meals, accommodation, "
            "transportation, daily_budget) VALUES ($1, $2, NULLIF($3, '')::date, $4, $5, $6, $7, $8, $9) "
            "RETURNING id",
            tripId, day.dayNumber, day.date, day.title, day.description, day.meals, day.accommodation,
            day.transportation, day.dailyBudget);
        const auto dayId = inserted[0][0].as<std::int64_t>();
        int seq = 1;
        for (const auto& place : day.places)
        {
            const auto category = isAllowedActivity(place.category) || place.category == "other" || place.category == "sightseeing"
                ? (place.category == "historical_places" ? "history" : place.category)
                : "other";
            std::string dbCategory = category;
            if (dbCategory == "museums") dbCategory = "culture";
            if (dbCategory == "beaches") dbCategory = "nature";
            if (dbCategory == "family") dbCategory = "family";
            if (dbCategory == "relaxation") dbCategory = "relaxation";
            tx.exec_params(
                "INSERT INTO itinerary_locations (itinerary_day_id, name, description, category, estimated_cost, "
                "visit_time, sequence_number, transportation_notes) VALUES ($1, $2, $3, $4, $5, $6, $7, $8)",
                dayId, place.name, place.description, dbCategory, place.baseCost, place.visitTime, seq,
                place.transport);
            ++seq;
        }
    }
}

http::Json itineraryJson(pqxx::nontransaction& tx, std::int64_t tripId)
{
    auto days = tx.exec_params(
        "SELECT id, day_number, date, title, description, meals, accommodation, transportation, daily_budget::text "
        "FROM itinerary_days WHERE trip_id = $1 ORDER BY day_number",
        tripId);
    http::Json list = http::Json::array();
    double estimated = 0;
    for (const auto& day : days)
    {
        auto locations = tx.exec_params(
            "SELECT name, description, category, estimated_cost::text, visit_time, sequence_number, transportation_notes "
            "FROM itinerary_locations WHERE itinerary_day_id = $1 ORDER BY sequence_number, id",
            day[0].as<std::int64_t>());
        http::Json places = http::Json::array();
        for (const auto& loc : locations)
        {
            places.push_back({
                {"name", loc[0].as<std::string>()},
                {"description", loc[1].is_null() ? "" : loc[1].as<std::string>()},
                {"category", loc[2].as<std::string>()},
                {"estimatedCost", std::stod(loc[3].as<std::string>("0"))},
                {"visitTime", loc[4].is_null() ? "" : loc[4].as<std::string>()},
                {"sequenceNumber", loc[5].as<int>()},
                {"transportationNotes", loc[6].is_null() ? "" : loc[6].as<std::string>()},
                {"estimated", true},
            });
        }
        const auto daily = std::stod(day[8].as<std::string>("0"));
        estimated += daily;
        list.push_back({
            {"day", day[1].as<int>()},
            {"date", day[2].is_null() ? "" : day[2].as<std::string>()},
            {"title", day[3].as<std::string>()},
            {"description", day[4].is_null() ? "" : day[4].as<std::string>()},
            {"meals", day[5].is_null() ? "" : day[5].as<std::string>()},
            {"accommodation", day[6].is_null() ? "" : day[6].as<std::string>()},
            {"transportation", day[7].is_null() ? "" : day[7].as<std::string>()},
            {"dailyBudget", daily},
            {"locations", places},
        });
    }
    return {{"days", list}, {"estimatedTotal", estimated}, {"estimated", true}};
}

PlannerInput toPlannerInput(const TripFields& fields)
{
    PlannerInput input;
    input.destination = fields.destination;
    input.startDate = fields.startDate;
    input.durationDays = fields.duration;
    input.budget = fields.budget;
    input.members = fields.members;
    input.travelStyle = fields.travelStyle;
    input.accommodationPreference = fields.accommodation;
    input.transportationPreference = fields.transportation;
    input.dietaryPreferences = fields.dietary;
    input.interests = fields.activities;
    return input;
}

PlannerInput inputFromRow(const pqxx::row& row, const std::vector<std::string>& activities)
{
    PlannerInput input;
    input.destination = row[3].as<std::string>();
    input.budget = std::stod(row[4].as<std::string>("0"));
    input.durationDays = row[5].as<int>();
    input.startDate = row[9].is_null() ? "" : row[9].as<std::string>();
    input.members = row[12].as<int>();
    input.travelStyle = row[13].as<std::string>();
    input.accommodationPreference = row[14].is_null() ? "" : row[14].as<std::string>();
    input.transportationPreference = row[15].is_null() ? "" : row[15].as<std::string>();
    input.dietaryPreferences = row[16].is_null() ? "" : row[16].as<std::string>();
    input.interests = activities;
    return input;
}
} // namespace

TripService::TripService(Database& database) : database_(database) {}

http::Json TripService::tripRowToJson(const pqxx::row& row, const std::vector<std::string>& activities) const
{
    http::Json interests = http::Json::array();
    for (const auto& item : activities) interests.push_back(item);
    return {
        {"id", row[0].as<std::int64_t>()},
        {"userId", row[1].as<std::int64_t>()},
        {"tripName", row[2].as<std::string>()},
        {"destination", row[3].as<std::string>()},
        {"budget", std::stod(row[4].as<std::string>("0"))},
        {"durationDays", row[5].as<int>()},
        {"activities", interests},
        {"createdAt", row[7].as<std::string>()},
        {"updatedAt", row[8].as<std::string>()},
        {"startDate", row[9].is_null() ? "" : row[9].as<std::string>()},
        {"endDate", row[10].is_null() ? "" : row[10].as<std::string>()},
        {"currency", row[11].as<std::string>()},
        {"members", row[12].as<int>()},
        {"travelStyle", row[13].as<std::string>()},
        {"accommodationPreference", row[14].is_null() ? "" : row[14].as<std::string>()},
        {"transportationPreference", row[15].is_null() ? "" : row[15].as<std::string>()},
        {"dietaryPreferences", row[16].is_null() ? "" : row[16].as<std::string>()},
        {"specialRequirements", row[17].is_null() ? "" : row[17].as<std::string>()},
        {"notes", row[18].is_null() ? "" : row[18].as<std::string>()},
        {"status", row[19].as<std::string>()},
    };
}

crow::response TripService::activities() const
{
    return http::json(200, {{"activities", kActivityCatalog}});
}

crow::response TripService::create(std::int64_t userId, const crow::request& req)
{
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok) return http::error(parsed.status, parsed.error);
    auto fields = parseTripBody(parsed.json, false);
    if (!fields.ok) return http::error(400, fields.error);

    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto inserted = tx.exec_params(
            "INSERT INTO trips (user_id, title, destination, budget, duration_days, interests, start_date, end_date, "
            "currency, members, travel_style, accommodation_preference, transportation_preference, dietary_preferences, "
            "special_requirements, notes, status) VALUES "
            "($1, $2, $3, $4, $5, $6::text[], NULLIF($7,'')::date, NULLIF($8,'')::date, $9, $10, $11, NULLIF($12,''), "
            "NULLIF($13,''), NULLIF($14,''), NULLIF($15,''), NULLIF($16,''), $17) "
            "RETURNING id, user_id, title, destination, budget::text, duration_days, interests, created_at, updated_at, "
            "start_date, end_date, currency, members, travel_style, accommodation_preference, transportation_preference, "
            "dietary_preferences, special_requirements, notes, status",
            userId, fields.title, fields.destination, fields.budget, fields.duration, pgTextArray(fields.activities),
            fields.startDate, fields.endDate, fields.currency, fields.members, fields.travelStyle, fields.accommodation,
            fields.transportation, fields.dietary, fields.special, fields.notes, fields.status);
        const auto tripId = inserted[0][0].as<std::int64_t>();
        saveActivities(tx, tripId, fields.activities);
        const auto plan = planner_.generate(toPlannerInput(fields));
        saveItinerary(tx, tripId, plan);
        tx.commit();

        pqxx::nontransaction read(*connection);
        auto itinerary = itineraryJson(read, tripId);
        auto body = tripRowToJson(inserted[0], fields.activities);
        body["itinerary"] = itinerary;
        body["budgetAllocation"] = plan.budgetAllocation;
        body["plannerSource"] = plan.source;
        body["plannerNotes"] = plan.notes;
        return http::json(201, {{"trip", body}});
    }
    catch (const DatabaseUnavailable&)
    {
        return http::error(503, "Database is unavailable");
    }
    catch (const std::exception& e)
    {
        CROW_LOG_ERROR << "Could not create the trip: " << e.what();
        return http::error(500, "Could not create the trip");
    }
}

crow::response TripService::list(std::int64_t userId)
{
    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);
        auto rows = tx.exec_params(std::string(kTripSelect) + " WHERE t.user_id = $1 ORDER BY t.created_at DESC", userId);
        http::Json trips = http::Json::array();
        for (const auto& row : rows)
        {
            auto acts = tx.exec_params("SELECT activity_name FROM trip_activities WHERE trip_id = $1 ORDER BY activity_name",
                                       row[0].as<std::int64_t>());
            std::vector<std::string> names;
            for (const auto& act : acts) names.push_back(act[0].as<std::string>());
            trips.push_back(tripRowToJson(row, names));
        }
        return http::json(200, {{"trips", trips}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not load trips");
    }
}

crow::response TripService::get(std::int64_t userId, const std::string& tripId)
{
    const auto id = jsonBody::asId(tripId);
    if (!id) return http::error(400, "Invalid trip id");
    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);
        auto rows = tx.exec_params(std::string(kTripSelect) + " WHERE t.id = $1 AND t.user_id = $2", *id, userId);
        if (rows.empty()) return http::error(404, "Trip not found");
        auto acts = tx.exec_params("SELECT activity_name FROM trip_activities WHERE trip_id = $1 ORDER BY activity_name", *id);
        std::vector<std::string> names;
        for (const auto& act : acts) names.push_back(act[0].as<std::string>());
        auto body = tripRowToJson(rows[0], names);
        body["itinerary"] = itineraryJson(tx, *id);
        return http::json(200, {{"trip", body}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not load the trip");
    }
}

crow::response TripService::update(std::int64_t userId, const std::string& tripId, const crow::request& req)
{
    const auto id = jsonBody::asId(tripId);
    if (!id) return http::error(400, "Invalid trip id");
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok) return http::error(parsed.status, parsed.error);

    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto existing = tx.exec_params(std::string(kTripSelect) + " WHERE t.id = $1 AND t.user_id = $2", *id, userId);
        if (existing.empty()) return http::error(404, "Trip not found");

        auto fields = parseTripBody(parsed.json, true);
        if (!fields.ok) return http::error(400, fields.error);
        if (fields.title.empty()) fields.title = existing[0][2].as<std::string>();
        if (fields.destination.empty()) fields.destination = existing[0][3].as<std::string>();
        if (!parsed.json.contains("budget") && !parsed.json.contains("totalBudget"))
            fields.budget = std::stod(existing[0][4].as<std::string>("0"));
        if (fields.duration < 1) fields.duration = existing[0][5].as<int>();
        if (fields.activities.empty())
        {
            auto acts = tx.exec_params("SELECT activity_name FROM trip_activities WHERE trip_id = $1", *id);
            for (const auto& act : acts) fields.activities.push_back(act[0].as<std::string>());
        }

        auto updated = tx.exec_params(
            "UPDATE trips SET title=$1, destination=$2, budget=$3, duration_days=$4, interests=$5::text[], "
            "start_date=NULLIF($6,'')::date, end_date=NULLIF($7,'')::date, currency=$8, members=$9, travel_style=$10, "
            "accommodation_preference=NULLIF($11,''), transportation_preference=NULLIF($12,''), "
            "dietary_preferences=NULLIF($13,''), special_requirements=NULLIF($14,''), notes=NULLIF($15,''), status=$16 "
            "WHERE id=$17 AND user_id=$18 "
            "RETURNING id, user_id, title, destination, budget::text, duration_days, interests, created_at, updated_at, "
            "start_date, end_date, currency, members, travel_style, accommodation_preference, transportation_preference, "
            "dietary_preferences, special_requirements, notes, status",
            fields.title, fields.destination, fields.budget, fields.duration, pgTextArray(fields.activities),
            fields.startDate.empty() ? (existing[0][9].is_null() ? "" : existing[0][9].as<std::string>()) : fields.startDate,
            fields.endDate.empty() ? (existing[0][10].is_null() ? "" : existing[0][10].as<std::string>()) : fields.endDate,
            fields.currency, fields.members, fields.travelStyle, fields.accommodation, fields.transportation,
            fields.dietary, fields.special, fields.notes, fields.status, *id, userId);
        saveActivities(tx, *id, fields.activities);
        tx.commit();
        return http::json(200, {{"trip", tripRowToJson(updated[0], fields.activities)}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not update the trip");
    }
}

crow::response TripService::remove(std::int64_t userId, const std::string& tripId)
{
    const auto id = jsonBody::asId(tripId);
    if (!id) return http::error(400, "Invalid trip id");
    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto result = tx.exec_params("DELETE FROM trips WHERE id = $1 AND user_id = $2 RETURNING id", *id, userId);
        if (result.empty()) return http::error(404, "Trip not found");
        tx.commit();
        return http::json(200, {{"ok", true}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not delete the trip");
    }
}

crow::response TripService::generate(std::int64_t userId, const std::string& tripId)
{
    const auto id = jsonBody::asId(tripId);
    if (!id) return http::error(400, "Invalid trip id");
    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto rows = tx.exec_params(std::string(kTripSelect) + " WHERE t.id = $1 AND t.user_id = $2", *id, userId);
        if (rows.empty()) return http::error(404, "Trip not found");
        auto acts = tx.exec_params("SELECT activity_name FROM trip_activities WHERE trip_id = $1", *id);
        std::vector<std::string> names;
        for (const auto& act : acts) names.push_back(act[0].as<std::string>());
        const auto plan = planner_.generate(inputFromRow(rows[0], names));
        saveItinerary(tx, *id, plan);
        tx.commit();
        pqxx::nontransaction read(*connection);
        auto itinerary = itineraryJson(read, *id);
        auto body = tripRowToJson(rows[0], names);
        body["itinerary"] = itinerary;
        body["budgetAllocation"] = plan.budgetAllocation;
        body["plannerSource"] = plan.source;
        body["plannerNotes"] = plan.notes;
        return http::json(200, {{"trip", body}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not generate the itinerary");
    }
}

crow::response TripService::itinerary(std::int64_t userId, const std::string& tripId)
{
    const auto id = jsonBody::asId(tripId);
    if (!id) return http::error(400, "Invalid trip id");
    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);
        auto rows = tx.exec_params("SELECT id FROM trips WHERE id = $1 AND user_id = $2", *id, userId);
        if (rows.empty()) return http::error(404, "Trip not found");
        return http::json(200, itineraryJson(tx, *id));
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not load the itinerary");
    }
}

crow::response TripService::replaceItinerary(std::int64_t userId, const std::string& tripId, const crow::request& req)
{
    const auto id = jsonBody::asId(tripId);
    if (!id) return http::error(400, "Invalid trip id");
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok) return http::error(parsed.status, parsed.error);
    if (!parsed.json.contains("days") || !parsed.json["days"].is_array())
    {
        return http::error(400, "days array is required");
    }

    PlannerResult plan;
    int number = 1;
    for (const auto& dayJson : parsed.json["days"])
    {
        PlannerDay day;
        day.dayNumber = dayJson.value("day", number);
        day.date = dayJson.value("date", "");
        day.title = jsonBody::trim(dayJson.value("title", "Day " + std::to_string(number)));
        day.description = dayJson.value("description", "");
        day.meals = dayJson.value("meals", "");
        day.accommodation = dayJson.value("accommodation", "");
        day.transportation = dayJson.value("transportation", "");
        if (dayJson.contains("locations") && dayJson["locations"].is_array())
        {
            for (const auto& loc : dayJson["locations"])
            {
                PlannerPlace place;
                place.name = jsonBody::trim(loc.value("name", ""));
                if (place.name.empty()) continue;
                place.description = loc.value("description", "");
                place.category = loc.value("category", "other");
                place.visitTime = loc.value("visitTime", "");
                place.baseCost = loc.value("estimatedCost", 0.0);
                place.transport = loc.value("transportationNotes", "");
                day.dailyBudget += place.baseCost;
                day.places.push_back(place);
            }
        }
        plan.days.push_back(day);
        ++number;
    }

    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto rows = tx.exec_params("SELECT id FROM trips WHERE id = $1 AND user_id = $2", *id, userId);
        if (rows.empty()) return http::error(404, "Trip not found");
        saveItinerary(tx, *id, plan);
        tx.commit();
        pqxx::nontransaction read(*connection);
        return http::json(200, itineraryJson(read, *id));
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not update the itinerary");
    }
}
