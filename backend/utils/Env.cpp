#include "utils/Env.h"

#include <cstdlib>
#include <stdexcept>

namespace env
{
std::optional<std::string> find(const std::string& name)
{
    const char* value = std::getenv(name.c_str());
    if (value == nullptr || *value == '\0')
    {
        return std::nullopt;
    }
    return std::string(value);
}

std::string get(const std::string& name, const std::string& fallback)
{
    return find(name).value_or(fallback);
}

std::string require(const std::string& name)
{
    auto value = find(name);
    if (!value)
    {
        throw std::runtime_error("Missing required environment variable " + name);
    }
    return *value;
}

int getInt(const std::string& name, int fallback)
{
    auto value = find(name);
    if (!value)
    {
        return fallback;
    }

    try
    {
        std::size_t consumed = 0;
        int parsed = std::stoi(*value, &consumed);
        if (consumed == value->size())
        {
            return parsed;
        }
    }
    catch (const std::exception&)
    {
    }
    throw std::runtime_error("Environment variable " + name + " must be an integer, got '" + *value + "'");
}
} // namespace env
