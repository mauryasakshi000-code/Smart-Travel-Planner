#pragma once

#include <crow.h>
#include <nlohmann/json.hpp>

#include <string>

namespace http
{
// Insertion-ordered so response keys appear in the order they are written.
using Json = nlohmann::ordered_json;

crow::response json(int status, const Json& body);

// Standard error body: { "error": "<message>" }
crow::response error(int status, const std::string& message);
} // namespace http
