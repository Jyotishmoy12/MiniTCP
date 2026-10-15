#include "net/byte_order.hpp"
#include "net/checksum.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
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

    void test_read_u16_be()
    {
        const std::array<std::byte, 2> bytes{
            std::byte{0x1F},
            std::byte{0x90},
        };

        const auto value = minitcp::net::read_u16_be(bytes);

        expect_true(value == 8080, "read_u16_be reads TCP port 8080");
    }

    void test_read_u32_be()
    {
        const std::array<std::byte, 4> bytes{
            std::byte{0xC0},
            std::byte{0xA8},
            std::byte{0x01},
            std::byte{0x14},
        };

        const auto value = minitcp::net::read_u32_be(bytes);

        expect_true(value == 0xC0A80114, "read_u32_be reads IPv4 address bytes");
    }

    void test_internet_checksum_even_length()
    {
        const std::array<std::byte, 4> bytes{
            std::byte{0x12},
            std::byte{0x34},
            std::byte{0x56},
            std::byte{0x78},
        };

        const auto checksum = minitcp::net::internet_checksum(bytes);

        expect_true(checksum == 0x9753, "internet_checksum handles even-length input");
    }

    void test_internet_checksum_odd_length()
    {
        const std::array<std::byte, 3> bytes{
            std::byte{0x12},
            std::byte{0x34},
            std::byte{0x56},
        };

        const auto checksum = minitcp::net::internet_checksum(bytes);

        expect_true(checksum == 0x97CB, "internet_checksum handles odd-length input");
    }

    void test_internet_checksum_wraparound()
    {
        const std::array<std::byte, 4> bytes{
            std::byte{0xFF},
            std::byte{0xFF},
            std::byte{0x00},
            std::byte{0x01},
        };

        const auto checksum = minitcp::net::internet_checksum(bytes);

        expect_true(checksum == 0xFFFE, "internet_checksum wraps carry bits");
    }
}

int main()
{
    test_read_u16_be();
    test_read_u32_be();
    test_internet_checksum_even_length();
    test_internet_checksum_odd_length();
    test_internet_checksum_wraparound();

    if (failures != 0)
    {
        std::cerr << failures << " test(s) failed" << '\n';
        return 1;
    }

    std::cout << "All MiniTCP tests passed" << '\n';
    return 0;
}