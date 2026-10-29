#include "services/ContentService.h"

#include "utils/Http.h"
#include "utils/JsonBody.h"

#include <optional>
#include <pqxx/pqxx>

namespace
{
http::Json storyJson(const pqxx::row& row)
{
    return {
        {"id", row[0].as<std::int64_t>()},
        {"userId", row[1].as<std::int64_t>()},
        {"tripId", row[2].is_null() ? http::Json(nullptr) : http::Json(row[2].as<std::int64_t>())},
        {"title", row[3].as<std::string>()},
        {"content", row[4].as<std::string>()},
        {"published", row[5].as<bool>()},
        {"coverImageUrl", row[6].is_null() ? "" : row[6].as<std::string>()},
        {"createdAt", row[7].as<std::string>()},
        {"updatedAt", row[8].as<std::string>()},
        {"author", row.size() > 9 && !row[9].is_null() ? row[9].as<std::string>() : ""},
        {"destination", row.size() > 10 && !row[10].is_null() ? row[10].as<std::string>() : ""},
    };
}
} // namespace

ContentService::ContentService(Database& database) : database_(database) {}

crow::response ContentService::listStories(std::int64_t userId, bool publicOnly)
{
    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);
        pqxx::result rows;
        if (publicOnly)
        {
            rows = tx.exec(
                "SELECT b.id, b.user_id, b.trip_id, b.title, b.content, b.published, b.cover_image_url, "
                "b.created_at, b.updated_at, u.full_name, t.destination "
                "FROM blogs b JOIN users u ON u.id = b.user_id "
                "LEFT JOIN trips t ON t.id = b.trip_id "
                "WHERE b.published = true ORDER BY b.published_at DESC NULLS LAST, b.created_at DESC");
        }
        else
        {
            rows = tx.exec_params(
                "SELECT b.id, b.user_id, b.trip_id, b.title, b.content, b.published, b.cover_image_url, "
                "b.created_at, b.updated_at, u.full_name, t.destination "
                "FROM blogs b JOIN users u ON u.id = b.user_id "
                "LEFT JOIN trips t ON t.id = b.trip_id "
                "WHERE b.user_id = $1 ORDER BY b.created_at DESC",
                userId);
        }
        http::Json list = http::Json::array();
        for (const auto& row : rows) list.push_back(storyJson(row));
        return http::json(200, {{"stories", list}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not load stories");
    }
}

crow::response ContentService::getStory(std::int64_t userId, const std::string& storyId, bool requireOwner)
{
    const auto id = jsonBody::asId(storyId);
    if (!id) return http::error(400, "Invalid story id");
    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);
        auto rows = tx.exec_params(
            "SELECT b.id, b.user_id, b.trip_id, b.title, b.content, b.published, b.cover_image_url, "
            "b.created_at, b.updated_at, u.full_name, t.destination "
            "FROM blogs b JOIN users u ON u.id = b.user_id LEFT JOIN trips t ON t.id = b.trip_id "
            "WHERE b.id = $1",
            *id);
        if (rows.empty()) return http::error(404, "Story not found");
        const auto owner = rows[0][1].as<std::int64_t>();
        const auto published = rows[0][5].as<bool>();
        if (requireOwner && owner != userId) return http::error(403, "You do not have access to this story");
        if (!requireOwner && !published && owner != userId) return http::error(404, "Story not found");
        return http::json(200, {{"story", storyJson(rows[0])}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not load the story");
    }
}

crow::response ContentService::createStory(std::int64_t userId, const crow::request& req)
{
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok) return http::error(parsed.status, parsed.error);
    const auto title = jsonBody::trim(parsed.json.value("title", ""));
    const auto content = jsonBody::trim(parsed.json.value("content", ""));
    const auto cover = jsonBody::trim(parsed.json.value("coverImageUrl", ""));
    const auto tripRaw = parsed.json.contains("tripId") && !parsed.json["tripId"].is_null()
        ? parsed.json["tripId"].dump()
        : "";
    if (title.empty() || content.empty()) return http::error(400, "Title and content are required");

    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        std::optional<std::int64_t> tripId;
        if (!tripRaw.empty() && tripRaw != "null")
        {
            auto parsedTrip = jsonBody::asId(parsed.json["tripId"].is_number() ? std::to_string(parsed.json["tripId"].get<std::int64_t>()) : jsonBody::trim(parsed.json["tripId"].get<std::string>()));
            if (!parsedTrip) return http::error(400, "Invalid trip id");
            auto owned = tx.exec_params("SELECT id FROM trips WHERE id = $1 AND user_id = $2", *parsedTrip, userId);
            if (owned.empty()) return http::error(400, "Trip not found for this account");
            tripId = parsedTrip;
        }
        pqxx::result inserted;
        if (tripId)
        {
            inserted = tx.exec_params(
                "INSERT INTO blogs (user_id, trip_id, title, content, published, cover_image_url) "
                "VALUES ($1, $2, $3, $4, false, NULLIF($5,'')) "
                "RETURNING id, user_id, trip_id, title, content, published, cover_image_url, created_at, updated_at",
                userId, *tripId, title, content, cover);
        }
        else
        {
            inserted = tx.exec_params(
                "INSERT INTO blogs (user_id, trip_id, title, content, published, cover_image_url) "
                "VALUES ($1, NULL, $2, $3, false, NULLIF($4,'')) "
                "RETURNING id, user_id, trip_id, title, content, published, cover_image_url, created_at, updated_at",
                userId, title, content, cover);
        }
        tx.commit();
        auto json = storyJson(inserted[0]);
        return http::json(201, {{"story", json}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not create the story");
    }
}

crow::response ContentService::updateStory(std::int64_t userId, const std::string& storyId, const crow::request& req)
{
    const auto id = jsonBody::asId(storyId);
    if (!id) return http::error(400, "Invalid story id");
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok) return http::error(parsed.status, parsed.error);
    const auto title = jsonBody::trim(parsed.json.value("title", ""));
    const auto content = jsonBody::trim(parsed.json.value("content", ""));
    const auto cover = jsonBody::trim(parsed.json.value("coverImageUrl", ""));
    if (title.empty() || content.empty()) return http::error(400, "Title and content are required");
    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto updated = tx.exec_params(
            "UPDATE blogs SET title=$1, content=$2, cover_image_url=NULLIF($3,'') "
            "WHERE id=$4 AND user_id=$5 "
            "RETURNING id, user_id, trip_id, title, content, published, cover_image_url, created_at, updated_at",
            title, content, cover, *id, userId);
        if (updated.empty()) return http::error(404, "Story not found");
        tx.commit();
        return http::json(200, {{"story", storyJson(updated[0])}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not update the story");
    }
}

crow::response ContentService::deleteStory(std::int64_t userId, const std::string& storyId)
{
    const auto id = jsonBody::asId(storyId);
    if (!id) return http::error(400, "Invalid story id");
    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto deleted = tx.exec_params("DELETE FROM blogs WHERE id = $1 AND user_id = $2 RETURNING id", *id, userId);
        if (deleted.empty()) return http::error(404, "Story not found");
        tx.commit();
        return http::json(200, {{"ok", true}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not delete the story");
    }
}

crow::response ContentService::publishStory(std::int64_t userId, const std::string& storyId)
{
    const auto id = jsonBody::asId(storyId);
    if (!id) return http::error(400, "Invalid story id");
    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto updated = tx.exec_params(
            "UPDATE blogs SET published = true, published_at = COALESCE(published_at, now()) "
            "WHERE id = $1 AND user_id = $2 "
            "RETURNING id, user_id, trip_id, title, content, published, cover_image_url, created_at, updated_at",
            *id, userId);
        if (updated.empty()) return http::error(404, "Story not found");
        tx.commit();
        return http::json(200, {{"story", storyJson(updated[0])}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not publish the story");
    }
}

crow::response ContentService::listSavedPlaces(std::int64_t userId)
{
    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);
        auto rows = tx.exec_params(
            "SELECT id, place_name, destination, notes, created_at FROM saved_places "
            "WHERE user_id = $1 ORDER BY created_at DESC",
            userId);
        http::Json list = http::Json::array();
        for (const auto& row : rows)
        {
            list.push_back({
                {"id", row[0].as<std::int64_t>()},
                {"placeName", row[1].as<std::string>()},
                {"destination", row[2].is_null() ? "" : row[2].as<std::string>()},
                {"notes", row[3].is_null() ? "" : row[3].as<std::string>()},
                {"createdAt", row[4].as<std::string>()},
            });
        }
        return http::json(200, {{"places", list}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not load saved places");
    }
}

crow::response ContentService::addSavedPlace(std::int64_t userId, const crow::request& req)
{
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok) return http::error(parsed.status, parsed.error);
    const auto name = jsonBody::trim(parsed.json.value("placeName", parsed.json.value("name", "")));
    const auto destination = jsonBody::trim(parsed.json.value("destination", ""));
    const auto notes = jsonBody::trim(parsed.json.value("notes", ""));
    if (name.empty()) return http::error(400, "Place name is required");
    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto inserted = tx.exec_params(
            "INSERT INTO saved_places (user_id, place_name, destination, notes) "
            "VALUES ($1, $2, NULLIF($3,''), NULLIF($4,'')) "
            "RETURNING id, place_name, destination, notes, created_at",
            userId, name, destination, notes);
        tx.commit();
        return http::json(201, {{"place", {
            {"id", inserted[0][0].as<std::int64_t>()},
            {"placeName", inserted[0][1].as<std::string>()},
            {"destination", inserted[0][2].is_null() ? "" : inserted[0][2].as<std::string>()},
            {"notes", inserted[0][3].is_null() ? "" : inserted[0][3].as<std::string>()},
            {"createdAt", inserted[0][4].as<std::string>()},
        }}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not save the place");
    }
}

crow::response ContentService::deleteSavedPlace(std::int64_t userId, const std::string& placeId)
{
    const auto id = jsonBody::asId(placeId);
    if (!id) return http::error(400, "Invalid place id");
    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto deleted = tx.exec_params("DELETE FROM saved_places WHERE id = $1 AND user_id = $2 RETURNING id", *id, userId);
        if (deleted.empty()) return http::error(404, "Place not found");
        tx.commit();
        return http::json(200, {{"ok", true}});
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not delete the place");
    }
}
