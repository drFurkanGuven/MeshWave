/**
 * @file config_store.cpp
 * @brief NVS-backed configuration via ESP Preferences.
 */
#include "config_store.h"
#include <Preferences.h>
#include <cstring>

namespace loralink::config {

namespace {

constexpr const char* kKeyDeviceName = "dev_name";
constexpr const char* kKeyDeviceId   = "dev_id";
constexpr const char* kKeyFreq       = "freq_hz";
constexpr const char* kKeySf         = "sf";
constexpr const char* kKeyBw         = "bw_hz";
constexpr const char* kKeyCr         = "cr";
constexpr const char* kKeySync       = "sync";
constexpr const char* kKeyTxPwr      = "tx_pwr";
constexpr const char* kKeyE220Ch     = "e220_ch";
constexpr const char* kKeyWifiSsid   = "wifi_ssid";
constexpr const char* kKeyWifiPass   = "wifi_pass";
constexpr const char* kKeyAesEn     = "aes_en";
constexpr const char* kKeyAckTmo     = "ack_tmo";
constexpr const char* kKeyMaxRetry   = "max_retry";
constexpr const char* kKeyConfigured = "configured";

} // namespace

ConfigStore::ConfigStore(const char* ns) : m_ns(ns) {}

void ConfigStore::loadDefaults(AppConfig& out, DeviceRole role) {
    std::memset(&out, 0, sizeof(out));
    out.device_id = (role == DeviceRole::Terminal) ? loralink::kCardputerId : loralink::kGatewayId;
    const char* name = (role == DeviceRole::Terminal) ? "Cardputer" : "Gateway";
    std::strncpy(out.device_name, name, sizeof(out.device_name) - 1);
    out.frequency_hz = LoRaDefaults::frequency_hz;
    out.spreading_factor = LoRaDefaults::spreading_factor;
    out.bandwidth_hz = LoRaDefaults::bandwidth_hz;
    out.coding_rate = LoRaDefaults::coding_rate;
    out.sync_word = LoRaDefaults::sync_word;
    out.tx_power_dbm = (role == DeviceRole::Terminal) ? 22 : LoRaDefaults::tx_power_dbm;
    out.e220_channel = LoRaDefaults::e220_channel;
    const char* ssid = (role == DeviceRole::Terminal)
        ? WifiDefaults::cardputer_ssid : WifiDefaults::gateway_ssid;
    std::strncpy(out.wifi_ssid, ssid, sizeof(out.wifi_ssid) - 1);
    std::strncpy(out.wifi_password, WifiDefaults::default_password,
                 sizeof(out.wifi_password) - 1);
    out.ack_timeout_ms = TransportDefaults::ack_timeout_ms;
    out.max_retries = TransportDefaults::max_retries;
}

bool ConfigStore::load(AppConfig& out) {
    Preferences prefs;
    if (!prefs.begin(m_ns, true)) return false;

    if (!prefs.getBool(kKeyConfigured, false)) {
        prefs.end();
        return false;
    }

    prefs.getString(kKeyDeviceName, out.device_name, sizeof(out.device_name));
    out.device_id = static_cast<DeviceId>(prefs.getUInt(kKeyDeviceId, out.device_id));
    out.frequency_hz = prefs.getUInt(kKeyFreq, out.frequency_hz);
    out.spreading_factor = prefs.getUChar(kKeySf, out.spreading_factor);
    out.bandwidth_hz = prefs.getUInt(kKeyBw, out.bandwidth_hz);
    out.coding_rate = prefs.getUChar(kKeyCr, out.coding_rate);
    out.sync_word = static_cast<uint16_t>(prefs.getUInt(kKeySync, out.sync_word));
    out.tx_power_dbm = static_cast<int8_t>(prefs.getChar(kKeyTxPwr, out.tx_power_dbm));
    out.e220_channel = prefs.getUChar(kKeyE220Ch, out.e220_channel);
    prefs.getString(kKeyWifiSsid, out.wifi_ssid, sizeof(out.wifi_ssid));
    prefs.getString(kKeyWifiPass, out.wifi_password, sizeof(out.wifi_password));
    out.aes_enabled = prefs.getBool(kKeyAesEn, false);
    out.ack_timeout_ms = prefs.getUInt(kKeyAckTmo, out.ack_timeout_ms);
    out.max_retries = prefs.getUChar(kKeyMaxRetry, out.max_retries);

    prefs.end();
    return true;
}

bool ConfigStore::save(const AppConfig& cfg) {
    Preferences prefs;
    if (!prefs.begin(m_ns, false)) return false;

    prefs.putString(kKeyDeviceName, cfg.device_name);
    prefs.putUInt(kKeyDeviceId, cfg.device_id);
    prefs.putUInt(kKeyFreq, cfg.frequency_hz);
    prefs.putUChar(kKeySf, cfg.spreading_factor);
    prefs.putUInt(kKeyBw, cfg.bandwidth_hz);
    prefs.putUChar(kKeyCr, cfg.coding_rate);
    prefs.putUInt(kKeySync, cfg.sync_word);
    prefs.putChar(kKeyTxPwr, cfg.tx_power_dbm);
    prefs.putUChar(kKeyE220Ch, cfg.e220_channel);
    prefs.putString(kKeyWifiSsid, cfg.wifi_ssid);
    prefs.putString(kKeyWifiPass, cfg.wifi_password);
    prefs.putBool(kKeyAesEn, cfg.aes_enabled);
    prefs.putUInt(kKeyAckTmo, cfg.ack_timeout_ms);
    prefs.putUChar(kKeyMaxRetry, cfg.max_retries);
    prefs.putBool(kKeyConfigured, true);

    prefs.end();
    return true;
}

bool ConfigStore::reset() {
    Preferences prefs;
    if (!prefs.begin(m_ns, false)) return false;
    prefs.clear();
    prefs.end();
    return true;
}

} // namespace loralink::config
