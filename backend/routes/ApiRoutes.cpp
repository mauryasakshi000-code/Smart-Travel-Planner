#include "routes/Routes.h"

#include "models/Types.h"
#include "services/AuthService.h"
#include "services/ContentService.h"
#include "services/DashboardService.h"
#include "services/ExpenseService.h"
#include "services/TripService.h"

namespace
{
template <typename Fn>
crow::response withUser(AuthService& auth, const crow::request& req, Fn&& fn)
{
    const auto user = auth.authenticate(req);
    if (!user)
    {
        return auth.unauthorized();
    }
    return fn(*user);
}
} // namespace

namespace routes
{
void registerApiRoutes(CrowApp& app, AuthService& auth, TripService& trips, ExpenseService& expenses,
                       DashboardService& dashboard, ContentService& content)
{
    CROW_ROUTE(app, "/api/auth/register").methods(crow::HTTPMethod::Post)([&auth](const crow::request& req) {
        return auth.registerUser(req);
    });
    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::Post)([&auth](const crow::request& req) {
        return auth.login(req);
    });
    CROW_ROUTE(app, "/api/auth/logout").methods(crow::HTTPMethod::Post)([&auth](const crow::request& req) {
        return auth.logout(req);
    });
    CROW_ROUTE(app, "/api/auth/me").methods(crow::HTTPMethod::Get)([&auth](const crow::request& req) {
        return auth.me(req);
    });

    CROW_ROUTE(app, "/api/activities").methods(crow::HTTPMethod::Get)([&trips]() { return trips.activities(); });

    CROW_ROUTE(app, "/api/dashboard/stats")
        .methods(crow::HTTPMethod::Get)([&auth, &dashboard](const crow::request& req) {
            return withUser(auth, req, [&](const UserRecord& user) { return dashboard.stats(user); });
        });

    CROW_ROUTE(app, "/api/trips").methods(crow::HTTPMethod::Get)([&auth, &trips](const crow::request& req) {
        return withUser(auth, req, [&](const UserRecord& user) { return trips.list(user.id); });
    });
    CROW_ROUTE(app, "/api/trips").methods(crow::HTTPMethod::Post)([&auth, &trips](const crow::request& req) {
        return withUser(auth, req, [&](const UserRecord& user) { return trips.create(user.id, req); });
    });

    CROW_ROUTE(app, "/api/trips/<string>/generate")
        .methods(crow::HTTPMethod::Post)([&auth, &trips](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return trips.generate(user.id, id); });
        });
    CROW_ROUTE(app, "/api/trips/<string>/itinerary")
        .methods(crow::HTTPMethod::Get)([&auth, &trips](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return trips.itinerary(user.id, id); });
        });
    CROW_ROUTE(app, "/api/trips/<string>/itinerary")
        .methods(crow::HTTPMethod::Put)([&auth, &trips](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return trips.replaceItinerary(user.id, id, req); });
        });
    CROW_ROUTE(app, "/api/trips/<string>/expenses")
        .methods(crow::HTTPMethod::Get)([&auth, &expenses](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return expenses.list(user.id, id); });
        });
    CROW_ROUTE(app, "/api/trips/<string>/expenses")
        .methods(crow::HTTPMethod::Post)([&auth, &expenses](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return expenses.add(user.id, id, req); });
        });

    CROW_ROUTE(app, "/api/trips/<string>")
        .methods(crow::HTTPMethod::Get)([&auth, &trips](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return trips.get(user.id, id); });
        });
    CROW_ROUTE(app, "/api/trips/<string>")
        .methods(crow::HTTPMethod::Put)([&auth, &trips](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return trips.update(user.id, id, req); });
        });
    CROW_ROUTE(app, "/api/trips/<string>")
        .methods(crow::HTTPMethod::Delete)([&auth, &trips](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return trips.remove(user.id, id); });
        });

    CROW_ROUTE(app, "/api/expenses/<string>")
        .methods(crow::HTTPMethod::Put)([&auth, &expenses](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return expenses.update(user.id, id, req); });
        });
    CROW_ROUTE(app, "/api/expenses/<string>")
        .methods(crow::HTTPMethod::Delete)([&auth, &expenses](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return expenses.remove(user.id, id); });
        });

    CROW_ROUTE(app, "/api/stories/public").methods(crow::HTTPMethod::Get)([&content]() {
        return content.listStories(0, true);
    });
    CROW_ROUTE(app, "/api/stories").methods(crow::HTTPMethod::Get)([&auth, &content](const crow::request& req) {
        return withUser(auth, req, [&](const UserRecord& user) { return content.listStories(user.id, false); });
    });
    CROW_ROUTE(app, "/api/stories").methods(crow::HTTPMethod::Post)([&auth, &content](const crow::request& req) {
        return withUser(auth, req, [&](const UserRecord& user) { return content.createStory(user.id, req); });
    });
    CROW_ROUTE(app, "/api/stories/<string>/publish")
        .methods(crow::HTTPMethod::Post)([&auth, &content](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return content.publishStory(user.id, id); });
        });
    CROW_ROUTE(app, "/api/stories/<string>")
        .methods(crow::HTTPMethod::Get)([&auth, &content](const crow::request& req, const std::string& id) {
            const auto user = auth.authenticate(req);
            return content.getStory(user ? user->id : 0, id, false);
        });
    CROW_ROUTE(app, "/api/stories/<string>")
        .methods(crow::HTTPMethod::Put)([&auth, &content](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return content.updateStory(user.id, id, req); });
        });
    CROW_ROUTE(app, "/api/stories/<string>")
        .methods(crow::HTTPMethod::Delete)([&auth, &content](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return content.deleteStory(user.id, id); });
        });

    CROW_ROUTE(app, "/api/places").methods(crow::HTTPMethod::Get)([&auth, &content](const crow::request& req) {
        return withUser(auth, req, [&](const UserRecord& user) { return content.listSavedPlaces(user.id); });
    });
    CROW_ROUTE(app, "/api/places").methods(crow::HTTPMethod::Post)([&auth, &content](const crow::request& req) {
        return withUser(auth, req, [&](const UserRecord& user) { return content.addSavedPlace(user.id, req); });
    });
    CROW_ROUTE(app, "/api/places/<string>")
        .methods(crow::HTTPMethod::Delete)([&auth, &content](const crow::request& req, const std::string& id) {
            return withUser(auth, req, [&](const UserRecord& user) { return content.deleteSavedPlace(user.id, id); });
        });
}
} // namespace routes
