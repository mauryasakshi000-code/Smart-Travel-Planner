#include "utils/Random.h"

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace randomBytes
{
std::string hex(std::size_t byteCount)
{
    std::vector<unsigned char> buffer(byteCount);
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    if (!urandom.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(byteCount)))
    {
        throw std::runtime_error("Could not read secure random bytes");
    }

    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned char byte : buffer)
    {
        out << std::setw(2) << static_cast<int>(byte);
    }
    return out.str();
}
} // namespace randomBytes
