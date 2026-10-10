#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace minitcp::net
{
    [[nodiscard]] inline std::uint16_t read_u16_be(std::span<const std::byte> bytes)
    {
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(bytes[0]) << 8) |
            static_cast<std::uint16_t>(bytes[1])
        );
    }

    [[nodiscard]] inline std::uint32_t read_u32_be(std::span<const std::byte> bytes)
    {
        return (static_cast<std::uint32_t>(bytes[0]) << 24) |
               (static_cast<std::uint32_t>(bytes[1]) << 16) |
               (static_cast<std::uint32_t>(bytes[2]) << 8) |
               static_cast<std::uint32_t>(bytes[3]);
    }
}