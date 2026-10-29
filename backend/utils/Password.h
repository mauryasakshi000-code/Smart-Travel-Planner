#pragma once

#include <string>

namespace password
{
std::string hashArgon2id(const std::string& plaintext);
bool verify(const std::string& plaintext, const std::string& encodedHash);
bool looksLikeArgon2(const std::string& encodedHash);
} // namespace password
