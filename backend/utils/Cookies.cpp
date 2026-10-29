#include "utils/Cookies.h"

#include <sstream>

namespace cookies
{
std::optional<std::string> get(const crow::request& req, const std::string& name)
{
    const auto header = req.get_header_value("Cookie");
    if (header.empty())
    {
        return std::nullopt;
    }

    std::size_t start = 0;
    while (start < header.size())
    {
        auto end = header.find(';', start);
        if (end == std::string::npos)
        {
            end = header.size();
        }
        auto piece = header.substr(start, end - start);
        auto eq = piece.find('=');
        if (eq != std::string::npos)
        {
            auto keyStart = piece.find_first_not_of(" \t");
            auto keyEnd = piece.find_last_not_of(" \t", eq - 1);
            if (keyStart != std::string::npos && keyEnd != std::string::npos && keyEnd >= keyStart)
            {
                auto key = piece.substr(keyStart, keyEnd - keyStart + 1);
                if (key == name)
                {
                    auto value = piece.substr(eq + 1);
                    auto vStart = value.find_first_not_of(" \t");
                    auto vEnd = value.find_last_not_of(" \t");
                    if (vStart == std::string::npos)
                    {
                        return std::string{};
                    }
                    return value.substr(vStart, vEnd - vStart + 1);
                }
            }
        }
        start = end + 1;
    }
    return std::nullopt;
}

std::string sessionSet(const std::string& token, int maxAgeSeconds, bool secure)
{
    std::ostringstream out;
    out << "stp_session=" << token
        << "; Path=/; HttpOnly; SameSite=Lax; Max-Age=" << maxAgeSeconds;
    if (secure)
    {
        out << "; Secure";
    }
    return out.str();
}

std::string sessionClear(bool secure)
{
    std::ostringstream out;
    out << "stp_session=; Path=/; HttpOnly; SameSite=Lax; Max-Age=0";
    if (secure)
    {
        out << "; Secure";
    }
    return out.str();
}
} // namespace cookies
