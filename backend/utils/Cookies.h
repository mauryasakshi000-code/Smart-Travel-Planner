#pragma once

#include <crow.h>

#include <optional>
#include <string>

namespace cookies
{
std::optional<std::string> get(const crow::request& req, const std::string& name);
std::string sessionSet(const std::string& token, int maxAgeSeconds, bool secure);
std::string sessionClear(bool secure);
} // namespace cookies
