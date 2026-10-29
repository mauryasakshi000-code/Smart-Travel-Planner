#pragma once

#include "AppConfig.h"

#include <pqxx/pqxx>

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

class DatabaseUnavailable : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

// Thread-safe PostgreSQL connection pool. pqxx::connection is not safe to share
// between threads, so each request borrows one exclusively via acquire().
// Connections are opened lazily up to DatabaseConfig::poolSize, and broken
// connections are discarded when returned so the pool heals after a DB restart.
class Database
{
public:
    class Connection
    {
    public:
        Connection(Database& owner, std::unique_ptr<pqxx::connection> connection);
        Connection(Connection&& other) noexcept;
        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
        Connection& operator=(Connection&&) = delete;
        ~Connection();

        pqxx::connection& operator*() { return *connection_; }
        pqxx::connection* operator->() { return connection_.get(); }

    private:
        Database* owner_;
        std::unique_ptr<pqxx::connection> connection_;
    };

    explicit Database(DatabaseConfig config);
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Opens the first connection, retrying while PostgreSQL finishes starting up.
    bool connectWithRetry();

    // Throws DatabaseUnavailable if no connection can be opened or none frees up in time.
    Connection acquire(std::chrono::milliseconds timeout = std::chrono::seconds(5));

    // Runs a trivial query; retries once so a stale pooled connection doesn't cause a false negative.
    bool isHealthy();

private:
    std::unique_ptr<pqxx::connection> openConnection() const;
    void release(std::unique_ptr<pqxx::connection> connection);

    DatabaseConfig config_;
    std::string connectionString_;

    std::mutex mutex_;
    std::condition_variable available_;
    std::vector<std::unique_ptr<pqxx::connection>> idle_;
    std::size_t openCount_ = 0;
};
