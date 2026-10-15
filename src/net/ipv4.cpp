#include "net/ipv4.hpp"

#include "net/byte_order.hpp"
#include "net/checksum.hpp"

#include <cstddef>
#include <cstdint>

namespace minitcp::net
{
    namespace
    {
        constexpr std::size_t minimum_header_size = 20;
        constexpr std::uint8_t supported_version = 4;
        constexpr std::uint8_t minimum_ihl = 5;
        constexpr std::uint16_t fragmentation_mask = 0x3FFF;

        Ipv4ParseResult failure(Ipv4ParseError error)
        {
            return Ipv4ParseResult{
                .header = std::nullopt,
                .error = error,
            };
        }
    }

    Ipv4ParseResult parse_ipv4_header(std::span<const std::byte> packet)
    {
        if (packet.size() < minimum_header_size)
        {
            return failure(Ipv4ParseError::packet_too_short);
        }

        const auto version_and_ihl =
            std::to_integer<std::uint8_t>(packet[0]);

        const auto version =
            static_cast<std::uint8_t>(version_and_ihl >> 4);

        const auto ihl =
            static_cast<std::uint8_t>(version_and_ihl & 0x0F);

        if (version != supported_version)
        {
            return failure(Ipv4ParseError::wrong_version);
        }

        if (ihl < minimum_ihl)
        {
            return failure(Ipv4ParseError::invalid_header_length);
        }

        if (ihl > minimum_ihl)
        {
            return failure(Ipv4ParseError::options_not_supported);
        }

        const auto header_length =
            static_cast<std::size_t>(ihl) * 4;

        const auto total_length =
            read_u16_be(packet.subspan(2, 2));

        if (total_length < header_length)
        {
            return failure(Ipv4ParseError::invalid_total_length);
        }

        if (packet.size() < total_length)
        {
            return failure(Ipv4ParseError::truncated_packet);
        }

        const auto flags_and_fragment_offset =
            read_u16_be(packet.subspan(6, 2));

        if ((flags_and_fragment_offset & fragmentation_mask) != 0)
        {
            return failure(Ipv4ParseError::fragmented_packet);
        }

        const auto header_bytes = packet.first(header_length);

        if (internet_checksum(header_bytes) != 0)
        {
            return failure(Ipv4ParseError::invalid_checksum);
        }

        const Ipv4Header header{
            .version = version,
            .header_length = static_cast<std::uint8_t>(header_length),
            .total_length = total_length,
            .identification = read_u16_be(packet.subspan(4, 2)),
            .flags_and_fragment_offset = flags_and_fragment_offset,
            .ttl = std::to_integer<std::uint8_t>(packet[8]),
            .protocol = std::to_integer<std::uint8_t>(packet[9]),
            .header_checksum = read_u16_be(packet.subspan(10, 2)),
            .source_address = read_u32_be(packet.subspan(12, 4)),
            .destination_address = read_u32_be(packet.subspan(16, 4)),
        };

        return Ipv4ParseResult{
            .header = header,
            .error = Ipv4ParseError::none,
        };
    }
}
