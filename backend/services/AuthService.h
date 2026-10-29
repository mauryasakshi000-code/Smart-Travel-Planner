#pragma once

#include "database/Database.h"
#include "models/Types.h"
#include "utils/Http.h"

#include <crow.h>

#include <optional>
#include <string>

class AuthService
{
public:
    AuthService(Database& database, int sessionSeconds, bool cookieSecure);

    crow::response registerUser(const crow::request& req);
    crow::response login(const crow::request& req);
    crow::response logout(const crow::request& req);
    crow::response me(const crow::request& req);

    std::optional<UserRecord> authenticate(const crow::request& req);
    crow::response unauthorized() const;
    crow::response forbidden() const;

private:
    Database& database_;
    int sessionSeconds_;
    bool cookieSecure_;

    crow::response withSessionCookie(int status, const http::Json& body, const std::string& token);
};
