# `include/net/byte_order.hpp`

## Purpose

This file contains helper functions for reading numbers from raw network packet bytes.

Network protocols like IPv4, ICMP, and TCP store multi-byte numbers in **network byte order**, also called **big-endian**.

That means the most important byte comes first.

Example:

```text
TCP port 8080 = 0x1F90

Stored in packet as:

byte[0] = 0x1F
byte[1] = 0x90
```

So we need helper functions that convert packet bytes into normal C++ integer values.

## Why This File Exists

Raw packets are just arrays of bytes.

For example, a TCP header starts like this:

```text
source port      2 bytes
destination port 2 bytes
sequence number  4 bytes
ack number       4 bytes
```

If we read these bytes in the wrong order, we get incorrect values.

Example:

```text
Correct big-endian:
[0x1F, 0x90] = 0x1F90 = 8080

Wrong little-endian:
[0x1F, 0x90] = 0x901F = 36895
```

So this file protects us from reading network packet fields incorrectly.

## Header Guard

```cpp
#pragma once
```

This tells the compiler to include this file only once per `.cpp` file.

Without this, the same function definitions could be included multiple times and cause compiler errors.

## Includes

```cpp
#include <cstddef>
#include <cstdint>
#include <span>
```

`<cstddef>` gives us:

```cpp
std::byte
```

We use `std::byte` because packet data is raw binary data, not text.

`<cstdint>` gives us fixed-size integer types:

```cpp
std::uint16_t
std::uint32_t
```

This matters because network fields have exact sizes.

`<span>` gives us:

```cpp
std::span
```

A `std::span` is a lightweight view over existing data. It does not copy packet bytes.

## Namespace

```cpp
namespace minitcp::net
{
}
```

This keeps networking helpers inside the MiniTCP networking namespace.

The full function names are:

```cpp
minitcp::net::read_u16_be
minitcp::net::read_u32_be
```

## `read_u16_be`

```cpp
[[nodiscard]] inline std::uint16_t read_u16_be(std::span<const std::byte> bytes)
```

This function reads 2 bytes as one unsigned 16-bit big-endian number.

`[[nodiscard]]` means the caller should not ignore the return value.

`inline` is used because this function is defined inside a header file.

`std::uint16_t` means the function returns a 16-bit unsigned integer.

`std::span<const std::byte> bytes` means the function receives a read-only view of bytes.

Example input:

```text
[0x1F, 0x90]
```

Step by step:

```text
bytes[0] = 0x1F
bytes[1] = 0x90
```

Move the first byte into the high position:

```text
0x1F << 8 = 0x1F00
```

Keep the second byte in the low position:

```text
0x90 = 0x0090
```

Combine them:

```text
0x1F00 | 0x0090 = 0x1F90
```

Final value:

```text
0x1F90 = 8080
```

So:

```cpp
read_u16_be([0x1F, 0x90])
```

returns:

```text
8080
```

## `read_u32_be`

```cpp
[[nodiscard]] inline std::uint32_t read_u32_be(std::span<const std::byte> bytes)
```

This function reads 4 bytes as one unsigned 32-bit big-endian number.

Example IPv4 address:

```text
192.168.1.20
```

In bytes:

```text
192 = 0xC0
168 = 0xA8
1   = 0x01
20  = 0x14
```

Packet bytes:

```text
[0xC0, 0xA8, 0x01, 0x14]
```

Step by step:

```text
0xC0 << 24 = 0xC0000000
0xA8 << 16 = 0x00A80000
0x01 << 8  = 0x00000100
0x14       = 0x00000014
```

Combine them:

```text
0xC0000000
| 0x00A80000
| 0x00000100
| 0x00000014
= 0xC0A80114
```

So:

```cpp
read_u32_be([0xC0, 0xA8, 0x01, 0x14])
```

returns:

```text
0xC0A80114
```

That is the 32-bit form of:

```text
192.168.1.20
```

## Where We Will Use This Later

We will use these helpers when parsing:

- IPv4 total length
- IPv4 source address
- IPv4 destination address
- ICMP identifier
- ICMP sequence number
- TCP source port
- TCP destination port
- TCP sequence number
- TCP acknowledgment number
- TCP window size

## Important Limitation

These functions assume the caller passes enough bytes.

For example:

```cpp
read_u16_be(...)
```

expects at least 2 bytes.

```cpp
read_u32_be(...)
```

expects at least 4 bytes.

Later, when we build IPv4 and TCP parsers, those parsers will check packet lengths before calling these helpers.
