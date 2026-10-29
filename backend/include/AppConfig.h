#pragma once

#include <string>

struct DatabaseConfig
{
    std::string host;
    int port = 5432;
    std::string name;
    std::string user;
    std::string password;
    int poolSize = 8;
    int connectTimeoutSeconds = 5;
    int startupRetries = 15;
    int startupRetryDelaySeconds = 2;

    // libpq keyword/value connection string with every value safely quoted.
    std::string connectionString() const;
};

struct ServerConfig
{
    int port = 8080;
    int threads = 4;
    std::string corsOrigin;
};

struct AppConfig
{
    ServerConfig server;
    DatabaseConfig database;

    // Throws std::runtime_error when a required variable is missing or malformed.
    static AppConfig fromEnvironment();
};
