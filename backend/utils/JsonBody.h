#pragma once

#include "utils/Http.h"

#include <crow.h>

#include <optional>
#include <string>
#include <utility>

namespace jsonBody
{
constexpr std::size_t kMaxBytes = 512 * 1024;

struct ParseResult
{
    bool ok = false;
    int status = 400;
    std::string error;
    http::Json json;
};

ParseResult parse(const crow::request& req);

std::string trim(const std::string& value);
std::string lower(std::string value);
bool validEmail(const std::string& email);
bool validPassword(const std::string& password, std::string& message);
std::optional<std::int64_t> asId(const std::string& value);
} // namespace jsonBody
