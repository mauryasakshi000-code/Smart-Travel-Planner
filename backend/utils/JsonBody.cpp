#include "utils/JsonBody.h"

#include <algorithm>
#include <cctype>
#include <regex>

namespace jsonBody
{
ParseResult parse(const crow::request& req)
{
    ParseResult result;
    if (req.body.size() > kMaxBytes)
    {
        result.status = 413;
        result.error = "Request body is too large";
        return result;
    }
    if (req.body.empty())
    {
        result.error = "JSON body is required";
        return result;
    }
    try
    {
        result.json = http::Json::parse(req.body);
        if (!result.json.is_object())
        {
            result.error = "JSON body must be an object";
            return result;
        }
        result.ok = true;
        result.status = 200;
        return result;
    }
    catch (const std::exception&)
    {
        result.error = "Invalid JSON body";
        return result;
    }
}

std::string trim(const std::string& value)
{
    auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return {};
    }
    auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

std::string lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool validEmail(const std::string& email)
{
    static const std::regex pattern(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)", std::regex::icase);
    return email.size() <= 255 && std::regex_match(email, pattern);
}

bool validPassword(const std::string& password, std::string& message)
{
    if (password.size() < 8 || password.size() > 72)
    {
        message = "Password must be between 8 and 72 characters";
        return false;
    }
    bool letter = false;
    bool digit = false;
    for (unsigned char c : password)
    {
        letter = letter || std::isalpha(c) != 0;
        digit = digit || std::isdigit(c) != 0;
    }
    if (!letter || !digit)
    {
        message = "Password must include at least one letter and one number";
        return false;
    }
    return true;
}

std::optional<std::int64_t> asId(const std::string& value)
{
    try
    {
        std::size_t idx = 0;
        const auto parsed = std::stoll(value, &idx, 10);
        if (idx != value.size() || parsed <= 0)
        {
            return std::nullopt;
        }
        return parsed;
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}
} // namespace jsonBody
