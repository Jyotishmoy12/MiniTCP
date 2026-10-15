# Internet Checksum

## Related Files

- `include/net/checksum.hpp`
- `src/net/checksum.cpp`

## Purpose

These files implement the Internet checksum algorithm used by IPv4, ICMP, and TCP to detect packet corruption.

The header declares the function:

```cpp
[[nodiscard]] std::uint16_t internet_checksum(std::span<const std::byte> bytes);
```

The source file contains its implementation. Keeping the declaration and implementation separate lets other parts of MiniTCP use the function without duplicating its code.

## Function Signature

```cpp
std::uint16_t internet_checksum(std::span<const std::byte> bytes)
```

- `std::uint16_t` is the 16-bit checksum returned to the caller.
- `internet_checksum` is the function name.
- `std::span<const std::byte>` is a read-only view of the packet bytes. It does not copy the packet.
- `[[nodiscard]]` asks the compiler to warn if the caller calculates a checksum and then ignores it.

There must be a space between the return type and function name:

```cpp
std::uint16_t internet_checksum(...)
```

Writing `std::uint16_t_internet_checksum(...)` would incorrectly create one unknown identifier instead of a return type followed by a function name.

## Basic Algorithm

The algorithm has four stages:

1. Read the input two bytes at a time.
2. Combine each byte pair into one big-endian 16-bit word.
3. Add the words using one's-complement carry wraparound.
4. Invert all bits of the final sum.

For these bytes:

```text
0x12 0x34 0x56 0x78
```

we create two 16-bit words:

```text
0x1234
0x5678
```

Add them:

```text
0x1234 + 0x5678 = 0x68AC
```

Invert the bits:

```text
~0x68AC = 0x9753
```

The checksum is therefore `0x9753`.

## Why the Sum Is 32 Bits

```cpp
std::uint32_t sum = 0;
```

Each packet word is 16 bits, but adding two words can produce a 17-bit result. A 32-bit accumulator keeps that carry bit long enough for us to wrap it correctly.

For example:

```text
0xFFFF + 0x0001 = 0x10000
```

## Reading Two Bytes at a Time

```cpp
for (std::size_t index = 0; index + 1 < bytes.size(); index += 2)
```

- `index += 2` advances one 16-bit word at a time.
- `index + 1 < bytes.size()` ensures that both bytes exist before accessing them.

The two bytes are combined in network byte order:

```cpp
const auto high = static_cast<std::uint16_t>(bytes[index]);
const auto low = static_cast<std::uint16_t>(bytes[index + 1]);
const auto word = static_cast<std::uint16_t>((high << 8) | low);
```

For `0x12 0x34`:

```text
high << 8 = 0x1200
low       = 0x0034
OR result = 0x1234
```

## Carry Wraparound

```cpp
if (sum > 0xFFFF)
{
    sum = (sum & 0xFFFF) + 1;
}
```

The Internet checksum uses one's-complement addition. An overflow carry is added back to the low 16 bits rather than discarded.

Example:

```text
sum              = 0x10000
low 16 bits      = 0x0000
wrapped carry    = 0x0001
new sum          = 0x0001
```

## Odd-Length Input

If there is one byte left, it becomes the high byte of the final word and an imaginary zero is used as the low byte:

```cpp
const auto high = static_cast<std::uint16_t>(bytes.back());
const auto word = static_cast<std::uint16_t>(high << 8);
```

For a final byte of `0x56`:

```text
0x56 becomes 0x5600
```

The zero byte is used only for the checksum calculation; it is not appended to the real packet.

## Final Bit Inversion

```cpp
return static_cast<std::uint16_t>(~sum & 0xFFFF);
```

- `~sum` flips every bit.
- `& 0xFFFF` keeps only the low 16 bits.
- `static_cast<std::uint16_t>` returns the required 16-bit value.

## How Validation Works

When creating a packet, its checksum field is set to zero before calculating and storing the checksum.

When validating a complete packet that already contains its checksum, calculating the checksum over the full protected data should produce:

```text
0x0000
```

We will use this behavior later when validating incoming IPv4 and ICMP packets.

## Tests

The current tests cover:

- even-length input;
- odd-length input;
- carry wraparound.

These cases exercise the three important paths through the algorithm before it is used by the IPv4 parser.
