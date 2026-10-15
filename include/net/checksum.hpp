#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace minitcp::net
{
    [[nodiscard]] std::uint16_t internet_checksum(std::span<const std::byte> bytes);
}
