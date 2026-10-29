#include "Server.h"

#include "routes/Routes.h"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>

namespace
{
int sessionSeconds()
{
    const char* v = std::getenv("SESSION_SECONDS");
    return v ? std::atoi(v) : 60 * 60 * 24 * 7; // default: 7 days
}

bool cookieSecure()
{
    const char* v = std::getenv("COOKIE_SECURE");
    return v && std::string(v) == "true"; // keep false on http://localhost
}
} // namespace

Server::Server(AppConfig config)
    : config_(std::move(config)),
      database_(config_.database),
      healthController_(database_),
      auth_(database_, sessionSeconds(), cookieSecure()),
      trips_(database_),
      expenses_(database_),
      dashboard_(database_),
      content_(database_)
{
}

int Server::run()
{
    if (!database_.connectWithRetry())
    {
        CROW_LOG_ERROR << "Starting without a database connection; /health will report it as disconnected";
    }

    configureCors();
    registerRoutes();

    CROW_LOG_INFO << "Smart Travel Planner backend listening on port " << config_.server.port
                  << " with " << config_.server.threads << " worker threads";

    app_.bindaddr("0.0.0.0")
        .port(static_cast<std::uint16_t>(config_.server.port))
        .concurrency(static_cast<std::uint16_t>(config_.server.threads))
        .run();

    return 0;
}

void Server::configureCors()
{
    app_.get_middleware<crow::CORSHandler>()
        .global()
        .origin(config_.server.corsOrigin)
        .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post, crow::HTTPMethod::Put,
                 crow::HTTPMethod::Delete, crow::HTTPMethod::Options)
        .headers("Content-Type", "Authorization")
        .allow_credentials();
}

void Server::registerRoutes()
{
    routes::registerHealthRoutes(app_, healthController_);
    routes::registerApiRoutes(app_, auth_, trips_, expenses_, dashboard_, content_);
    routes::registerFallbackRoute(app_);
}