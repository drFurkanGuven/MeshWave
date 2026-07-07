/**
 * @file e220_driver.cpp
 * @brief E220-400T22S UART driver — M5 LoRa E220 library + transparent P2P.
 */
#if defined(HAS_E220) && HAS_E220

#include "e220_driver.h"
#include "../config/defaults.h"
#include "../../common/logger.h"
#include "M5_LoRa_E220.h"
#include <cstring>

namespace loralink::radio {

using loralink::Logger;
using loralink::config::LoRaDefaults;

struct E220Driver::Impl {
    HardwareSerial serial{2};
    LoRa_E220 module;
    LoRaConfigItem_t e220{};
};

namespace {

uint8_t airRateFromBandwidth(uint32_t bw_hz) {
    if (bw_hz <= 125000) return DATA_RATE_2_4Kbps;
    if (bw_hz <= 250000) return DATA_RATE_9_6Kbps;
    return DATA_RATE_19_2Kbps;
}

uint8_t channelFromFrequency(uint32_t freq_hz) {
    // E220: 410.125 MHz + channel * 1 MHz
    float mhz = static_cast<float>(freq_hz) / 1000000.0f;
    int ch = static_cast<int>(mhz - 410.125f + 0.5f);
    if (ch < 0) ch = 0;
    if (ch > 83) ch = 83;
    return static_cast<uint8_t>(ch);
}

uint8_t txPowerEnum(int8_t dbm) {
    if (dbm >= 20) return TX_POWER_22dBm;
    if (dbm >= 15) return TX_POWER_17dBm;
    if (dbm >= 12) return TX_POWER_13dBm;
    return TX_POWER_10dBm;
}

void fillE220Defaults(LoRaConfigItem_t& c, const RadioConfig& rc) {
    c.own_address = 0x0000;
    c.baud_rate = BAUD_9600;
    c.uart_config = UART_8N1;
    c.air_data_rate = airRateFromBandwidth(rc.bandwidth_hz);
    c.subpacket_size = SUBPACKET_200_BYTE;
    c.rssi_ambient_noise_flag = RSSI_AMBIENT_NOISE_DISABLE;
    c.transmitting_power = txPowerEnum(rc.tx_power_dbm);
    c.own_channel = rc.e220_channel ? rc.e220_channel : channelFromFrequency(rc.frequency_hz);
    c.rssi_byte_flag = RSSI_BYTE_DISABLE;
    c.transmission_method_type = UART_P2P_MODE;
    c.lbt_flag = LBT_DISABLE;
    c.wor_cycle = WOR_2000MS;
    c.encryption_key = 0x0000;
    c.target_address = 0x0000;
    c.target_channel = c.own_channel;
}

} // namespace

E220Driver::E220Driver(int tx_pin, int rx_pin, uint32_t uart_baud)
    : m_impl(new Impl()), m_tx(tx_pin), m_rx(rx_pin), m_baud(uart_baud) {
    m_mutex = xSemaphoreCreateMutex();
    m_cfg.frequency_hz = LoRaDefaults::frequency_hz;
    m_cfg.spreading_factor = LoRaDefaults::spreading_factor;
    m_cfg.bandwidth_hz = LoRaDefaults::bandwidth_hz;
    m_cfg.coding_rate = LoRaDefaults::coding_rate;
    m_cfg.sync_word = LoRaDefaults::sync_word;
    m_cfg.tx_power_dbm = 22;
    m_cfg.preamble_length = LoRaDefaults::preamble_length;
    m_cfg.e220_channel = LoRaDefaults::e220_channel;
}

E220Driver::~E220Driver() {
    if (m_mutex) vSemaphoreDelete(m_mutex);
    delete m_impl;
}

bool E220Driver::lock(uint32_t timeout_ms) {
    return m_mutex && xSemaphoreTake(m_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void E220Driver::unlock() {
    if (m_mutex) xSemaphoreGive(m_mutex);
}

bool E220Driver::applyModuleConfig() {
    fillE220Defaults(m_impl->e220, m_cfg);
    m_impl->module.SetDefaultConfigValue(m_impl->e220);
    fillE220Defaults(m_impl->e220, m_cfg);

    int rc = m_impl->module.InitLoRaSetting(m_impl->e220);
    if (rc != 0) {
        Logger::warning("E220", "InitLoRaSetting failed (%d). DIP M0/M1=11 for config?", rc);
        Logger::warning("E220", "Using existing module settings (DIP M0/M1=00)");
        return false;
    }
    Logger::info("E220", "Module configured ch=0x%02X air=%u",
                 m_impl->e220.own_channel, m_impl->e220.air_data_rate);
    m_configured = true;
    return true;
}

bool E220Driver::initialize() {
    if (!lock()) return false;

    m_impl->serial.begin(m_baud, SERIAL_8N1, m_rx, m_tx);
    m_impl->module.Init(&m_impl->serial, CONFIG_MODE_BAUD, SERIAL_8N1, m_rx, m_tx);

    fillE220Defaults(m_impl->e220, m_cfg);
    applyModuleConfig(); // non-fatal if DIP not in config mode

    m_ready = true;
    Logger::info("E220", "UART %lu baud TX=%d RX=%d", m_baud, m_tx, m_rx);
    unlock();
    return true;
}

bool E220Driver::setConfig(const RadioConfig& cfg) {
    if (!lock()) return false;
    m_cfg = cfg;
    if (m_ready) applyModuleConfig();
    unlock();
    return true;
}

bool E220Driver::getConfig(RadioConfig& cfg) const {
    cfg = m_cfg;
    return true;
}

bool E220Driver::sendPacket(const uint8_t* data, size_t len) {
    if (!data || len == 0 || len > 200 || !m_ready) return false;
    if (!lock()) return false;

    fillE220Defaults(m_impl->e220, m_cfg);
    int rc = m_impl->module.SendFrame(m_impl->e220, const_cast<uint8_t*>(data),
                                      static_cast<int>(len));
    if (rc == 0) {
        Logger::debug("E220", "TX %u bytes", static_cast<unsigned>(len));
        Logger::dumpHex(LogLevel::Debug, "E220", data, len);
    } else {
        Logger::error("E220", "SendFrame failed (%d)", rc);
    }
    unlock();
    return rc == 0;
}

bool E220Driver::receivePacket(RadioPacket& out, uint32_t timeout_ms) {
    if (!m_ready) return false;

    const uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        if (!lock(10)) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        RecvFrame_t frame{};
        int rc = m_impl->module.RecieveFrame(&frame);
        unlock();

        if (rc == 0 && frame.recv_data_len > 0) {
            out.length = frame.recv_data_len;
            if (out.length > kMaxRadioFrame) out.length = kMaxRadioFrame;
            std::memcpy(out.data.data(), frame.recv_data, out.length);
            out.meta.rssi = frame.rssi;
            out.meta.snr = 0;
            out.meta.rx_timestamp_ms = millis();
            m_last_rssi = frame.rssi;
            Logger::debug("E220", "RX %u bytes RSSI %d", out.length, frame.rssi);
            Logger::dumpHex(LogLevel::Debug, "E220", out.data.data(), out.length);
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    return false;
}

bool E220Driver::sleep() {
    Logger::debug("E220", "sleep() — use DIP/WOR on module hardware");
    return true;
}

bool E220Driver::wake() {
    Logger::debug("E220", "wake()");
    return m_ready;
}

int16_t E220Driver::readRSSI() {
    return m_last_rssi;
}

bool E220Driver::isChannelFree() {
    if (!lock(5)) return true;
    bool free = m_impl->serial.available() == 0;
    unlock();
    return free;
}

} // namespace loralink::radio

#endif // HAS_E220
