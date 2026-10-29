#include "utils/Password.h"

#include <argon2.h>

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace password
{
namespace
{
constexpr std::size_t kHashLen = 32;
constexpr std::size_t kSaltLen = 16;
constexpr uint32_t kTimeCost = 3;
constexpr uint32_t kMemoryKib = 64 * 1024;
constexpr uint32_t kParallelism = 1;

std::vector<uint8_t> randomSalt()
{
    std::vector<uint8_t> salt(kSaltLen);
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    if (!urandom.read(reinterpret_cast<char*>(salt.data()), static_cast<std::streamsize>(kSaltLen)))
    {
        throw std::runtime_error("Could not read salt bytes");
    }
    return salt;
}
} // namespace

std::string hashArgon2id(const std::string& plaintext)
{
    const auto salt = randomSalt();
    const auto encodedLen =
        argon2_encodedlen(kTimeCost, kMemoryKib, kParallelism, kSaltLen, kHashLen, Argon2_id);
    std::vector<char> encoded(encodedLen + 1, '\0');

    const int rc = argon2id_hash_encoded(
        kTimeCost, kMemoryKib, kParallelism, plaintext.data(), plaintext.size(), salt.data(), kSaltLen,
        kHashLen, encoded.data(), encodedLen);

    if (rc != ARGON2_OK)
    {
        throw std::runtime_error(std::string("Password hashing failed: ") + argon2_error_message(rc));
    }
    return std::string(encoded.data());
}

bool looksLikeArgon2(const std::string& encodedHash)
{
    return encodedHash.rfind("$argon2id$", 0) == 0 || encodedHash.rfind("$argon2i$", 0) == 0
        || encodedHash.rfind("$argon2d$", 0) == 0;
}

bool verify(const std::string& plaintext, const std::string& encodedHash)
{
    if (!looksLikeArgon2(encodedHash))
    {
        return false;
    }
    return argon2id_verify(encodedHash.c_str(), plaintext.data(), plaintext.size()) == ARGON2_OK;
}
} // namespace password
