# IPv4 Header Parsing

## Related Files

- `include/net/ipv4.hpp`
- `src/net/ipv4.cpp`
- `tests/test_ipv4.cpp`

## Purpose

These files safely parse the fixed portion of an incoming IPv4 header.

The parser does not trust packet data. It validates the packet before exposing fields such as the source address, destination address, protocol, and total length.

The MVP accepts only:

- IPv4;
- a standard 20-byte header;
- complete, non-fragmented packets;
- packets with a valid header checksum.

IPv4 options and fragmentation are deliberately rejected for now.

## Header and Source Separation

`ipv4.hpp` contains the public interface:

- the possible parse errors;
- the parsed header structure;
- the result structure;
- the parser declaration.

`ipv4.cpp` contains the implementation.

Other MiniTCP components include the header without needing to know how parsing is implemented internally.

## The First IPv4 Byte

The first byte contains two separate four-bit values:

```text
  7 6 5 4 3 2 1 0
+-------+-------+
|Version|  IHL  |
+-------+-------+
```

For a normal IPv4 header, the byte is `0x45`:

```text
0x45 = 0100 0101
       ---- ----
         4    5
```

The version is extracted by shifting right:

```cpp
const auto version =
    static_cast<std::uint8_t>(version_and_ihl >> 4);
```

```text
0100 0101 >> 4 = 0000 0100 = 4
```

The IHL is extracted with a mask:

```cpp
const auto ihl =
    static_cast<std::uint8_t>(version_and_ihl & 0x0F);
```

```text
0100 0101
0000 1111
--------- AND
0000 0101 = 5
```

## Internet Header Length

IHL is measured in 32-bit words, not bytes.

Therefore:

```text
IHL 5 * 4 bytes = 20 bytes
```

The parser rejects values below 5 because an IPv4 header cannot be shorter than 20 bytes.

Values above 5 indicate IPv4 options. Options are outside the MVP, so the parser rejects them explicitly.

## Safe Length Validation

The parser first checks that at least 20 bytes exist:

```cpp
if (packet.size() < minimum_header_size)
```

This must happen before reading any IPv4 fields. It prevents out-of-bounds memory access on malformed or truncated input.

The header's `total_length` field is then validated:

```cpp
if (total_length < header_length)
```

A packet cannot claim to be shorter than its own header.

The parser also checks:

```cpp
if (packet.size() < total_length)
```

This rejects packets that claim more bytes than were actually received.

## Fragmentation Check

The flags and fragment offset share one 16-bit field:

```text
3 flag bits + 13 fragment-offset bits
```

The mask is:

```cpp
constexpr std::uint16_t fragmentation_mask = 0x3FFF;
```

`0x3FFF` selects the More Fragments flag and all fragment-offset bits. It does not select the Don't Fragment flag.

Therefore this normal value is accepted:

```text
0x4000 = Don't Fragment
```

But this value is rejected:

```text
0x2000 = More Fragments
```

MiniTCP does not yet reassemble multiple fragments, so processing them would produce incomplete transport-layer data.

## Header Checksum Validation

Only the IPv4 header is covered by the IPv4 checksum:

```cpp
const auto header_bytes = packet.first(header_length);
```

For a complete header that already contains its checksum:

```cpp
internet_checksum(header_bytes)
```

must return `0x0000`.

A nonzero result means at least one protected header bit differs from what the sender checksummed, so the packet is rejected.

## Parsed Header Structure

After all validation succeeds, the parser builds an `Ipv4Header`:

```cpp
const Ipv4Header header{
    .version = version,
    .header_length = static_cast<std::uint8_t>(header_length),
    .total_length = total_length,
    // Remaining fields...
};
```

This uses C++20 designated initializers. Naming each member makes packet-field mapping easier to audit than relying only on member order.

Multi-byte fields are read with the big-endian helpers from `byte_order.hpp`.

## Parse Result

The parser does not throw exceptions for malformed network input. Invalid packets are expected in networking.

Instead, it returns:

```cpp
struct Ipv4ParseResult
{
    std::optional<Ipv4Header> header;
    Ipv4ParseError error;
};
```

On success:

```text
header contains a value
error is none
```

On failure:

```text
header is empty
error explains the rejection
```

The explicit Boolean conversion lets callers write:

```cpp
if (!result)
{
    // Reject or log the packet.
}
```

## Test Packet Construction

The test creates a real 20-byte IPv4 header with its checksum bytes initially set to zero.

It calculates the checksum:

```cpp
const auto checksum = minitcp::net::internet_checksum(packet);
```

Then it stores the checksum in network byte order:

```cpp
packet[10] = static_cast<std::byte>((checksum >> 8) & 0xFF);
packet[11] = static_cast<std::byte>(checksum & 0xFF);
```

The high byte goes first because IPv4 uses network byte order.

## Current Tests

The IPv4 tests verify that:

- a valid header is accepted;
- important fields are decoded correctly;
- a packet shorter than 20 bytes is rejected;
- a non-IPv4 version is rejected;
- fragmented input is rejected;
- a corrupted checksum is rejected.

This gives us a validated IPv4 boundary before adding ICMP processing.
