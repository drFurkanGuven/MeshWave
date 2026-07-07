#pragma once

/**
 * @file radio_packet.h
 * @brief Raw radio frame container and RX metadata.
 */

#include "../../common/types.h"
#include <cstdint>
#include <array>

namespace loralink::radio {

struct RxMetadata {
    int16_t  rssi{0};
    int8_t   snr{0};
    uint32_t rx_timestamp_ms{0};
};

struct RadioPacket {
    std::array<uint8_t, kMaxRadioFrame> data{};
    size_t   length{0};
    RxMetadata meta{};
};

} // namespace loralink::radio
