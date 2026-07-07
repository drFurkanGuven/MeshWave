#pragma once

/**
 * @file crc16.h
 * @brief CRC-16/CCITT-FALSE for packet validation.
 */

#include <cstddef>
#include <cstdint>

namespace loralink::protocol {

/**
 * @brief Compute CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF).
 */
uint16_t crc16(const uint8_t* data, size_t len);

inline bool verifyCrc(const uint8_t* data, size_t len, uint16_t expected) {
    return crc16(data, len) == expected;
}

} // namespace loralink::protocol
