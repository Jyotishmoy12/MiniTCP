#include "net/checksum.hpp"

namespace minitcp::net
{
    std::uint16_t internet_checksum(std::span<const std::byte> bytes)
    {
        std::uint32_t sum = 0;

        for (std::size_t index = 0; index + 1 < bytes.size(); index += 2)
        {
            const auto high = static_cast<std::uint16_t>(bytes[index]);
            const auto low = static_cast<std::uint16_t>(bytes[index + 1]);
            const auto word = static_cast<std::uint16_t>((high << 8) | low);
            sum += word;

            if (sum > 0xFFFF)
            {
                sum = (sum & 0xFFFF) + 1;
            }
        }
        if (bytes.size() % 2 != 0)
        {
            const auto high = static_cast<std::uint16_t>(bytes.back());
            const auto word = static_cast<std::uint16_t>(high << 8);
            sum += word;

            if (sum > 0xFFFF)
            {
                sum = (sum & 0xFFFF) + 1;
            }
        }
        return static_cast<std::uint16_t>(~sum & 0xFFFF);
    }
}
