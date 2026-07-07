#pragma once

/**
 * @file packet_types.h
 * @brief LoRaLink wire protocol constants and enums.
 */

#include <cstdint>
#include <cstddef>

namespace loralink::protocol {

constexpr uint16_t kMagic       = 0x4B4C; // "LK" little-endian
constexpr uint8_t  kVersion     = 0x01;
constexpr size_t   kHeaderSize  = 19;
constexpr size_t   kCrcSize     = 2;
constexpr size_t   kMaxWireSize = kHeaderSize + 180 + kCrcSize;

enum class PacketType : uint8_t {
    Hello      = 0x01,
    Ping       = 0x02,
    Pong       = 0x03,
    Message    = 0x10,
    Ack        = 0x11,
    Nack       = 0x12,
    DeviceInfo = 0x20,
    KeepAlive  = 0x30,
    Error      = 0x40,
    ConfigSync = 0x50,
};

enum PacketFlags : uint8_t {
    FlagAckReq    = 1 << 0,
    FlagFragment  = 1 << 1,
    FlagLastFrag  = 1 << 2,
    FlagEncrypted = 1 << 3,
    FlagBroadcast = 1 << 4,
};

} // namespace loralink::protocol
