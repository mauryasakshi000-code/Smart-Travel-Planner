#include "AppConfig.h"

#include "utils/Env.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace
{
std::string quoteConnectionValue(const std::string& value)
{
    std::string quoted = "'";
    for (char c : value)
    {
        if (c == '\\' || c == '\'')
        {
            quoted += '\\';
        }
        quoted += c;
    }
    quoted += '\'';
    return quoted;
}

int requirePositive(const std::string& name, int value)
{
    if (value <= 0)
    {
        throw std::runtime_error("Environment variable " + name + " must be greater than zero");
    }
    return value;
}
} // namespace

std::string DatabaseConfig::connectionString() const
{
    std::ostringstream out;
    out << "host=" << quoteConnectionValue(host)
        << " port=" << port
        << " dbname=" << quoteConnectionValue(name)
        << " user=" << quoteConnectionValue(user)
        << " password=" << quoteConnectionValue(password)
        << " connect_timeout=" << connectTimeoutSeconds
        << " application_name='smart-travel-backend'";
    return out.str();
}

AppConfig AppConfig::fromEnvironment()
{
    AppConfig config;

    const int defaultThreads = static_cast<int>(std::max(2u, std::thread::hardware_concurrency()));
    config.server.port = requirePositive("SERVER_PORT", env::getInt("SERVER_PORT", 8080));
    config.server.threads = requirePositive("SERVER_THREADS", env::getInt("SERVER_THREADS", defaultThreads));
    config.server.corsOrigin = env::get("CORS_ORIGIN", "http://localhost:5173");

    config.database.host = env::get("DB_HOST", "localhost");
    config.database.port = requirePositive("DB_PORT", env::getInt("DB_PORT", 5432));
    config.database.name = env::require("DB_NAME");
    config.database.user = env::require("DB_USER");
    config.database.password = env::require("DB_PASSWORD");
    config.database.poolSize = requirePositive("DB_POOL_SIZE", env::getInt("DB_POOL_SIZE", 8));
    config.database.connectTimeoutSeconds =
        requirePositive("DB_CONNECT_TIMEOUT", env::getInt("DB_CONNECT_TIMEOUT", 5));
    config.database.startupRetries =
        requirePositive("DB_STARTUP_RETRIES", env::getInt("DB_STARTUP_RETRIES", 15));

    return config;
}
