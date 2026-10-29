#include "services/AuthService.h"

#include "utils/Cookies.h"
#include "utils/JsonBody.h"
#include "utils/Password.h"
#include "utils/Random.h"

#include <pqxx/pqxx>

AuthService::AuthService(Database& database, int sessionSeconds, bool cookieSecure)
    : database_(database), sessionSeconds_(sessionSeconds), cookieSecure_(cookieSecure)
{
}

crow::response AuthService::unauthorized() const
{
    return http::error(401, "Authentication required");
}

crow::response AuthService::forbidden() const
{
    return http::error(403, "You do not have access to this resource");
}

std::optional<UserRecord> AuthService::authenticate(const crow::request& req)
{
    const auto token = cookies::get(req, "stp_session");
    if (!token || token->empty())
    {
        return std::nullopt;
    }

    try
    {
        auto connection = database_.acquire();
        pqxx::nontransaction tx(*connection);
        auto result = tx.exec_params(
            "SELECT u.id, u.full_name, u.email, u.created_at "
            "FROM sessions s JOIN users u ON u.id = s.user_id "
            "WHERE s.session_token = $1 AND s.expires_at > now()",
            *token);
        if (result.empty())
        {
            return std::nullopt;
        }
        UserRecord user;
        user.id = result[0][0].as<std::int64_t>();
        user.fullName = result[0][1].as<std::string>();
        user.email = result[0][2].as<std::string>();
        user.createdAt = result[0][3].as<std::string>();
        return user;
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

crow::response AuthService::withSessionCookie(int status, const http::Json& body, const std::string& token)
{
    auto res = http::json(status, body);
    res.set_header("Set-Cookie", cookies::sessionSet(token, sessionSeconds_, cookieSecure_));
    return res;
}

crow::response AuthService::registerUser(const crow::request& req)
{
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok)
    {
        return http::error(parsed.status, parsed.error);
    }

    const auto fullName = jsonBody::trim(parsed.json.value("fullName", parsed.json.value("full_name", "")));
    const auto email = jsonBody::lower(jsonBody::trim(parsed.json.value("email", "")));
    const auto passwordValue = parsed.json.value("password", "");
    const auto confirm = parsed.json.value("confirmPassword", parsed.json.value("confirm_password", passwordValue));

    if (fullName.size() < 2 || fullName.size() > 100)
    {
        return http::error(400, "Full name must be between 2 and 100 characters");
    }
    if (!jsonBody::validEmail(email))
    {
        return http::error(400, "Enter a valid email address");
    }
    std::string passwordMessage;
    if (!jsonBody::validPassword(passwordValue, passwordMessage))
    {
        return http::error(400, passwordMessage);
    }
    if (passwordValue != confirm)
    {
        return http::error(400, "Password and confirmation do not match");
    }

    try
    {
        const auto hash = password::hashArgon2id(passwordValue);
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto existing = tx.exec_params("SELECT 1 FROM users WHERE email = $1", email);
        if (!existing.empty())
        {
            return http::error(409, "An account with this email already exists");
        }
        auto inserted = tx.exec_params(
            "INSERT INTO users (full_name, email, password_hash) "
            "VALUES ($1, $2, $3) RETURNING id, full_name, email, created_at",
            fullName, email, hash);
        tx.commit();
        UserRecord user;
        user.id = inserted[0][0].as<std::int64_t>();
        user.fullName = inserted[0][1].as<std::string>();
        user.email = inserted[0][2].as<std::string>();
        user.createdAt = inserted[0][3].as<std::string>();
        return http::json(201, {{"user", userJson(user)}, {"message", "Account created. You can log in now."}});
    }
    catch (const pqxx::unique_violation&)
    {
        return http::error(409, "An account with this email already exists");
    }
    catch (const DatabaseUnavailable&)
    {
        return http::error(503, "Database is unavailable");
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not create the account");
    }
}

crow::response AuthService::login(const crow::request& req)
{
    const auto parsed = jsonBody::parse(req);
    if (!parsed.ok)
    {
        return http::error(parsed.status, parsed.error);
    }
    const auto email = jsonBody::lower(jsonBody::trim(parsed.json.value("email", "")));
    const auto passwordValue = parsed.json.value("password", "");
    if (email.empty() || passwordValue.empty())
    {
        return http::error(400, "Email and password are required");
    }

    try
    {
        auto connection = database_.acquire();
        pqxx::work tx(*connection);
        auto result = tx.exec_params(
            "SELECT id, full_name, email, password_hash, created_at FROM users WHERE email = $1", email);
        if (result.empty() || !password::verify(passwordValue, result[0][3].as<std::string>()))
        {
            return http::error(401, "Invalid email or password");
        }

        UserRecord user;
        user.id = result[0][0].as<std::int64_t>();
        user.fullName = result[0][1].as<std::string>();
        user.email = result[0][2].as<std::string>();
        user.createdAt = result[0][4].as<std::string>();

        const auto token = randomBytes::hex(32);
        tx.exec_params(
            "INSERT INTO sessions (user_id, session_token, expires_at) "
            "VALUES ($1, $2, now() + ($3 * interval '1 second'))",
            user.id, token, sessionSeconds_);
        tx.commit();
        return withSessionCookie(200, {{"user", userJson(user)}}, token);
    }
    catch (const DatabaseUnavailable&)
    {
        return http::error(503, "Database is unavailable");
    }
    catch (const std::exception&)
    {
        return http::error(500, "Could not log in");
    }
}

crow::response AuthService::logout(const crow::request& req)
{
    if (const auto token = cookies::get(req, "stp_session"))
    {
        try
        {
            auto connection = database_.acquire();
            pqxx::work tx(*connection);
            tx.exec_params("DELETE FROM sessions WHERE session_token = $1", *token);
            tx.commit();
        }
        catch (const std::exception&)
        {
        }
    }
    auto res = http::json(200, {{"ok", true}});
    res.set_header("Set-Cookie", cookies::sessionClear(cookieSecure_));
    return res;
}

crow::response AuthService::me(const crow::request& req)
{
    const auto user = authenticate(req);
    if (!user)
    {
        return unauthorized();
    }
    return http::json(200, {{"user", userJson(*user)}});
}
