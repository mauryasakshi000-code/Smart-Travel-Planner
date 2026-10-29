#pragma once

#include "database/Database.h"

#include <crow.h>
#include <cstdint>
#include <string>

class ContentService
{
public:
    explicit ContentService(Database& database);

    crow::response listStories(std::int64_t userId, bool publicOnly);
    crow::response getStory(std::int64_t userId, const std::string& storyId, bool requireOwner);
    crow::response createStory(std::int64_t userId, const crow::request& req);
    crow::response updateStory(std::int64_t userId, const std::string& storyId, const crow::request& req);
    crow::response deleteStory(std::int64_t userId, const std::string& storyId);
    crow::response publishStory(std::int64_t userId, const std::string& storyId);

    crow::response listSavedPlaces(std::int64_t userId);
    crow::response addSavedPlace(std::int64_t userId, const crow::request& req);
    crow::response deleteSavedPlace(std::int64_t userId, const std::string& placeId);

private:
    Database& database_;
};
