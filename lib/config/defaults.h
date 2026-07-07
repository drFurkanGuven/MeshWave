#pragma once

/**
 * @file defaults.h
 * @brief Factory default configuration values.
 */

#include "../common/types.h"

namespace loralink::config {

struct LoRaDefaults {
    static constexpr uint32_t frequency_hz     = 433125000;
    static constexpr uint8_t  spreading_factor = 12;
    static constexpr uint32_t bandwidth_hz     = 125000;
    static constexpr uint8_t  coding_rate      = 5;   // 4/5
    static constexpr uint16_t sync_word        = 0x0012;
    static constexpr int8_t   tx_power_dbm     = 14;
    static constexpr uint16_t preamble_length  = 8;
    static constexpr uint8_t  e220_channel     = 0x17;
};

struct TransportDefaults {
    static constexpr uint32_t ack_timeout_ms   = 3000;
    static constexpr uint8_t  max_retries      = 3;
    static constexpr size_t   dedup_window     = 64;
};

struct WifiDefaults {
    static constexpr const char* gateway_ssid     = "LoRaLink-GW";
    static constexpr const char* cardputer_ssid     = "LoRaLink-CP";
    static constexpr const char* default_password   = "loralink123";
    static constexpr const char* static_ip          = "192.168.4.1";
};

} // namespace loralink::config
