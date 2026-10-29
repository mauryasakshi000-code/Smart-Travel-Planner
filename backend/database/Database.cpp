#include "database/Database.h"

#include <crow/logging.h>

#include <thread>
#include <utility>

Database::Connection::Connection(Database& owner, std::unique_ptr<pqxx::connection> connection)
    : owner_(&owner), connection_(std::move(connection))
{
}

Database::Connection::Connection(Connection&& other) noexcept
    : owner_(other.owner_), connection_(std::move(other.connection_))
{
    other.owner_ = nullptr;
}

Database::Connection::~Connection()
{
    if (owner_ != nullptr && connection_)
    {
        owner_->release(std::move(connection_));
    }
}

Database::Database(DatabaseConfig config)
    : config_(std::move(config)), connectionString_(config_.connectionString())
{
}

bool Database::connectWithRetry()
{
    for (int attempt = 1; attempt <= config_.startupRetries; ++attempt)
    {
        try
        {
            auto connection = acquire();
            CROW_LOG_INFO << "Connected to PostgreSQL database '" << connection->dbname() << "' at "
                          << config_.host << ":" << config_.port
                          << " (server version " << connection->server_version() << ")";
            return true;
        }
        catch (const std::exception& e)
        {
            CROW_LOG_WARNING << "PostgreSQL connection attempt " << attempt << "/" << config_.startupRetries
                             << " failed: " << e.what();
        }

        if (attempt < config_.startupRetries)
        {
            std::this_thread::sleep_for(std::chrono::seconds(config_.startupRetryDelaySeconds));
        }
    }
    return false;
}

Database::Connection Database::acquire(std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::unique_lock<std::mutex> lock(mutex_);

    while (true)
    {
        while (!idle_.empty())
        {
            auto connection = std::move(idle_.back());
            idle_.pop_back();
            if (connection->is_open())
            {
                return Connection(*this, std::move(connection));
            }
            --openCount_;
        }

        if (openCount_ < static_cast<std::size_t>(config_.poolSize))
        {
            // Reserve the slot, then connect without holding the lock.
            ++openCount_;
            lock.unlock();
            try
            {
                return Connection(*this, openConnection());
            }
            catch (const std::exception& e)
            {
                lock.lock();
                --openCount_;
                available_.notify_one();
                throw DatabaseUnavailable(std::string("Could not connect to PostgreSQL: ") + e.what());
            }
        }

        if (available_.wait_until(lock, deadline) == std::cv_status::timeout)
        {
            throw DatabaseUnavailable("Timed out waiting for a free database connection");
        }
    }
}

bool Database::isHealthy()
{
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        try
        {
            auto connection = acquire(std::chrono::seconds(2));
            pqxx::nontransaction tx(*connection);
            auto result = tx.exec("SELECT 1");
            return result.size() == 1;
        }
        catch (const pqxx::broken_connection& e)
        {
            CROW_LOG_WARNING << "Health check hit a broken connection, retrying: " << e.what();
        }
        catch (const std::exception& e)
        {
            CROW_LOG_WARNING << "Health check failed: " << e.what();
            return false;
        }
    }
    return false;
}

std::unique_ptr<pqxx::connection> Database::openConnection() const
{
    return std::make_unique<pqxx::connection>(connectionString_);
}

void Database::release(std::unique_ptr<pqxx::connection> connection)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection->is_open())
        {
            idle_.push_back(std::move(connection));
        }
        else
        {
            --openCount_;
        }
    }
    available_.notify_one();
}
