#pragma once

/**
 * @file config_store.h
 * @brief NVS-backed persistent configuration via ESP Preferences.
 */

#include "defaults.h"
#include "../common/types.h"
#include <cstdint>
#include <array>
#include <string>

namespace loralink::config {

/**
 * @struct AppConfig
 * @brief Complete device configuration snapshot.
 */
struct AppConfig {
    char     device_name[kMaxDeviceName]{};
    DeviceId device_id{kCardputerId};

    // LoRa PHY
    uint32_t frequency_hz{LoRaDefaults::frequency_hz};
    uint8_t  spreading_factor{LoRaDefaults::spreading_factor};
    uint32_t bandwidth_hz{LoRaDefaults::bandwidth_hz};
    uint8_t  coding_rate{LoRaDefaults::coding_rate};
    uint16_t sync_word{LoRaDefaults::sync_word};
    int8_t   tx_power_dbm{LoRaDefaults::tx_power_dbm};
    uint8_t  e220_channel{LoRaDefaults::e220_channel};

    // WiFi
    char wifi_ssid[33]{};
    char wifi_password[65]{};

    // Security
    bool     aes_enabled{false};
    std::array<uint8_t, 16> aes_key{};

    // Transport
    uint32_t ack_timeout_ms{TransportDefaults::ack_timeout_ms};
    uint8_t  max_retries{TransportDefaults::max_retries};
};

/**
 * @class ConfigStore
 * @brief Load/save AppConfig to NVS namespace "loralink".
 */
class ConfigStore {
public:
    explicit ConfigStore(const char* ns = "loralink");

    bool load(AppConfig& out);
    bool save(const AppConfig& cfg);
    void loadDefaults(AppConfig& out, DeviceRole role);
    bool reset();

private:
    const char* m_ns;
};

} // namespace loralink::config
