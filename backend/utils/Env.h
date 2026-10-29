#pragma once

#include <optional>
#include <string>

namespace env
{
// Empty values are treated the same as unset variables.
std::optional<std::string> find(const std::string& name);

std::string get(const std::string& name, const std::string& fallback);

// Throws std::runtime_error if the variable is missing.
std::string require(const std::string& name);

// Throws std::runtime_error if the variable is set but is not an integer.
int getInt(const std::string& name, int fallback);
} // namespace env
