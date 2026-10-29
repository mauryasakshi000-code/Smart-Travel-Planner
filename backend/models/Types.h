#pragma once

#include "utils/Http.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct UserRecord
{
    std::int64_t id = 0;
    std::string fullName;
    std::string email;
    std::string createdAt;
};

inline http::Json userJson(const UserRecord& user)
{
    return {
        {"id", user.id},
        {"fullName", user.fullName},
        {"email", user.email},
        {"createdAt", user.createdAt},
    };
}

const std::vector<std::string> kAllowedActivities = {
    "sightseeing", "historical_places", "museums", "nature", "beaches", "adventure",
    "shopping",    "food",              "nightlife", "culture", "relaxation", "family",
    "history",     "photography",
};

const std::vector<http::Json> kActivityCatalog = {
    {{"id", "sightseeing"}, {"label", "Sightseeing"}},
    {{"id", "historical_places"}, {"label", "Historical places"}},
    {{"id", "museums"}, {"label", "Museums"}},
    {{"id", "nature"}, {"label", "Nature and hiking"}},
    {{"id", "beaches"}, {"label", "Beaches"}},
    {{"id", "adventure"}, {"label", "Adventure sports"}},
    {{"id", "shopping"}, {"label", "Shopping"}},
    {{"id", "food"}, {"label", "Food tours"}},
    {{"id", "nightlife"}, {"label", "Nightlife"}},
    {{"id", "culture"}, {"label", "Cultural experiences"}},
    {{"id", "relaxation"}, {"label", "Relaxation"}},
    {{"id", "family"}, {"label", "Family activities"}},
    {{"id", "history"}, {"label", "History"}},
    {{"id", "photography"}, {"label", "Photography"}},
};

inline bool isAllowedActivity(const std::string& value)
{
    return std::find(kAllowedActivities.begin(), kAllowedActivities.end(), value) != kAllowedActivities.end();
}
