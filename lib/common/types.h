#pragma once

/**
 * @file types.h
 * @brief Core value types used across LoRaLink layers.
 */

#include <cstdint>
#include <cstddef>
#include <array>
#include <string>

namespace loralink {

using DeviceId    = uint16_t;
using SequenceNum = uint16_t;
using Timestamp   = uint32_t;

constexpr DeviceId    kBroadcastId   = 0xFFFF;
constexpr DeviceId    kCardputerId   = 0x0001;
constexpr DeviceId    kGatewayId     = 0x0002;
constexpr SequenceNum kInvalidSeq    = 0xFFFF;

constexpr size_t kMaxPayloadSize   = 180;
constexpr size_t kMaxRadioFrame    = 255;
constexpr size_t kMaxChatText      = 500;
constexpr size_t kMaxDeviceName    = 32;
constexpr size_t kMaxMessageStore  = 1000;

enum class DeviceRole : uint8_t {
    Terminal = 0,
    Gateway  = 1,
};

enum class MessageDirection : uint8_t {
    Inbound  = 0,
    Outbound = 1,
};

/**
 * @brief Result type for operations that can fail without exceptions.
 */
template<typename T>
struct Result {
    bool     ok{false};
    T        value{};
    int      error{0};

    static Result success(T v) {
        return {true, v, 0};
    }
    static Result failure(int code) {
        return {false, {}, code};
    }
    explicit operator bool() const { return ok; }
};

struct VoidResult {
    bool ok{false};
    int  error{0};

    static VoidResult success() { return {true, 0}; }
    static VoidResult failure(int code) { return {false, code}; }
    explicit operator bool() const { return ok; }
};

} // namespace loralink
