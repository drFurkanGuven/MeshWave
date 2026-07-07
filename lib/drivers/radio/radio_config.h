#pragma once

/**
 * @file radio_config.h
 * @brief Radio PHY configuration shared by all drivers.
 */

#include <cstdint>

namespace loralink::radio {

struct RadioConfig {
    uint32_t frequency_hz{433125000};
    uint8_t  spreading_factor{12};
    uint32_t bandwidth_hz{125000};
    uint8_t  coding_rate{5};
    uint16_t sync_word{0x0012};
    int8_t   tx_power_dbm{14};
    uint16_t preamble_length{8};
    uint8_t  e220_channel{0x17};
};

} // namespace loralink::radio
