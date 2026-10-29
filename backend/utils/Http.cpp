#include "utils/Http.h"

namespace http
{
crow::response json(int status, const Json& body)
{
    crow::response res(status, body.dump(-1, ' ', false, Json::error_handler_t::replace));
    res.set_header("Content-Type", "application/json");
    return res;
}

crow::response error(int status, const std::string& message)
{
    return json(status, {{"error", message}});
}
} // namespace http
