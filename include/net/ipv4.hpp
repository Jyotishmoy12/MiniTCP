#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace minitcp::net
{
    enum class Ipv4ParseError
    {
        none,
        packet_too_short,
        wrong_version,
        invalid_header_length,
        options_not_supported,
        invalid_total_length,
        truncated_packet,
        fragmented_packet,
        invalid_checksum,
    };

    struct Ipv4Header
    {
        std::uint8_t version;
        std::uint8_t header_length;
        std::uint16_t total_length;
        std::uint16_t identification;
        std::uint16_t flags_and_fragment_offset;
        std::uint8_t ttl;
        std::uint8_t protocol;
        std::uint16_t header_checksum;
        std::uint32_t source_address;
        std::uint32_t destination_address;
    };

    struct Ipv4ParseResult
    {
        std::optional<Ipv4Header> header;
        Ipv4ParseError error;

        [[nodiscard]] explicit operator bool() const
        {
            return header.has_value();
        }
    };

    [[nodiscard]] Ipv4ParseResult parse_ipv4_header(
        std::span<const std::byte> packet
    );
}
