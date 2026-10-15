#include "net/checksum.hpp"
#include "net/ipv4.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>

namespace
{
    int failures = 0;

    void expect_true(bool condition, std::string_view message)
    {
        if (!condition)
        {
            ++failures;
            std::cerr << "[FAIL] " << message << '\n';
            return;
        }

        std::cout << "[PASS] " << message << '\n';
    }

    std::array<std::byte, 20> make_valid_ipv4_header()
    {
        std::array<std::byte, 20> packet{
            std::byte{0x45}, // IPv4, 20-byte header
            std::byte{0x00}, // DSCP and ECN
            std::byte{0x00}, std::byte{0x14}, // Total length: 20
            std::byte{0x12}, std::byte{0x34}, // Identification
            std::byte{0x40}, std::byte{0x00}, // Don't fragment
            std::byte{0x40}, // TTL: 64
            std::byte{0x01}, // Protocol: ICMP
            std::byte{0x00}, std::byte{0x00}, // Checksum placeholder
            std::byte{0xC0}, std::byte{0xA8},
            std::byte{0x01}, std::byte{0x14}, // 192.168.1.20
            std::byte{0x0A}, std::byte{0x4D},
            std::byte{0x00}, std::byte{0x02}, // 10.77.0.2
        };

        const auto checksum =
            minitcp::net::internet_checksum(packet);

        packet[10] = static_cast<std::byte>(
            (checksum >> 8) & 0xFF
        );

        packet[11] = static_cast<std::byte>(
            checksum & 0xFF
        );

        return packet;
    }

    void test_valid_ipv4_header()
    {
        const auto packet = make_valid_ipv4_header();
        const auto result = minitcp::net::parse_ipv4_header(packet);

        expect_true(
            static_cast<bool>(result),
            "valid IPv4 header is accepted"
        );

        if (!result)
        {
            return;
        }

        expect_true(
            result.header->version == 4,
            "IPv4 version is parsed"
        );

        expect_true(
            result.header->header_length == 20,
            "IPv4 header length is parsed"
        );

        expect_true(
            result.header->total_length == 20,
            "IPv4 total length is parsed"
        );

        expect_true(
            result.header->protocol == 1,
            "IPv4 ICMP protocol is parsed"
        );

        expect_true(
            result.header->source_address == 0xC0A80114,
            "IPv4 source address is parsed"
        );

        expect_true(
            result.header->destination_address == 0x0A4D0002,
            "IPv4 destination address is parsed"
        );
    }

    void test_short_packet_is_rejected()
    {
        const std::array<std::byte, 19> packet{};

        const auto result =
            minitcp::net::parse_ipv4_header(packet);

        expect_true(
            result.error ==
                minitcp::net::Ipv4ParseError::packet_too_short,
            "short IPv4 packet is rejected"
        );
    }

    void test_wrong_version_is_rejected()
    {
        auto packet = make_valid_ipv4_header();
        packet[0] = std::byte{0x65};

        const auto result =
            minitcp::net::parse_ipv4_header(packet);

        expect_true(
            result.error ==
                minitcp::net::Ipv4ParseError::wrong_version,
            "non-IPv4 packet is rejected"
        );
    }

    void test_fragmented_packet_is_rejected()
    {
        auto packet = make_valid_ipv4_header();
        packet[6] = std::byte{0x20};

        const auto result =
            minitcp::net::parse_ipv4_header(packet);

        expect_true(
            result.error ==
                minitcp::net::Ipv4ParseError::fragmented_packet,
            "fragmented IPv4 packet is rejected"
        );
    }

    void test_invalid_checksum_is_rejected()
    {
        auto packet = make_valid_ipv4_header();
        packet[8] = std::byte{0x3F};

        const auto result =
            minitcp::net::parse_ipv4_header(packet);

        expect_true(
            result.error ==
                minitcp::net::Ipv4ParseError::invalid_checksum,
            "invalid IPv4 checksum is rejected"
        );
    }
}

int main()
{
    test_valid_ipv4_header();
    test_short_packet_is_rejected();
    test_wrong_version_is_rejected();
    test_fragmented_packet_is_rejected();
    test_invalid_checksum_is_rejected();

    if (failures != 0)
    {
        std::cerr << failures << " test(s) failed" << '\n';
        return 1;
    }

    std::cout << "All IPv4 tests passed" << '\n';
    return 0;
}