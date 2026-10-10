#include "net/byte_order.hpp"

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
}

int main()
{
    test_read_u16_be();
    test_read_u32_be();

    if (failures != 0)
    {
        std::cerr << failures << " test(s) failed" << '\n';
        return 1;
    }

    std::cout << "All MiniTCP tests passed" << '\n';
    return 0;
}