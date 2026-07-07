/**
 * @file sx1278_driver.cpp
 * @brief SX1278 SPI driver for LilyGO T3 V1.6.1 using LoRa.h.
 */
#if defined(HAS_SX1278) && HAS_SX1278

#include "sx1278_driver.h"
#include "../config/defaults.h"
#include "../../common/logger.h"
#include <SPI.h>
#include <LoRa.h>

namespace loralink::radio {

using loralink::Logger;
using loralink::config::LoRaDefaults;

Sx1278Driver::Sx1278Driver(int cs, int rst, int dio0, int sck, int miso, int mosi)
    : m_cs(cs), m_rst(rst), m_dio0(dio0), m_sck(sck), m_miso(miso), m_mosi(mosi) {
    m_mutex = xSemaphoreCreateMutex();
    m_cfg.frequency_hz = LoRaDefaults::frequency_hz;
    m_cfg.spreading_factor = LoRaDefaults::spreading_factor;
    m_cfg.bandwidth_hz = LoRaDefaults::bandwidth_hz;
    m_cfg.coding_rate = LoRaDefaults::coding_rate;
    m_cfg.sync_word = LoRaDefaults::sync_word;
    m_cfg.tx_power_dbm = LoRaDefaults::tx_power_dbm;
    m_cfg.preamble_length = LoRaDefaults::preamble_length;
}

Sx1278Driver::~Sx1278Driver() {
    if (m_mutex) vSemaphoreDelete(m_mutex);
}

bool Sx1278Driver::lock(uint32_t timeout_ms) {
    return m_mutex && xSemaphoreTake(m_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void Sx1278Driver::unlock() {
    if (m_mutex) xSemaphoreGive(m_mutex);
}

bool Sx1278Driver::applyChipConfig() {
    SPI.begin(m_sck, m_miso, m_mosi, m_cs);
    LoRa.setPins(m_cs, m_rst, m_dio0);

    long freq = static_cast<long>(m_cfg.frequency_hz);
    if (!LoRa.begin(freq)) {
        Logger::error("SX1278", "LoRa.begin(%ld) failed", freq);
        return false;
    }

    LoRa.setSpreadingFactor(m_cfg.spreading_factor);
    LoRa.setSignalBandwidth(static_cast<long>(m_cfg.bandwidth_hz));
    LoRa.setCodingRate4(m_cfg.coding_rate);
    LoRa.setSyncWord(static_cast<int>(m_cfg.sync_word & 0xFF));
    LoRa.setPreambleLength(m_cfg.preamble_length);
    LoRa.setTxPower(m_cfg.tx_power_dbm);
    LoRa.enableCrc();

    Logger::info("SX1278", "RF %.3f MHz SF%u BW%lu CR4/%u sync=0x%02X TX%ddBm",
                 m_cfg.frequency_hz / 1000000.0,
                 m_cfg.spreading_factor,
                 static_cast<unsigned long>(m_cfg.bandwidth_hz),
                 m_cfg.coding_rate,
                 m_cfg.sync_word & 0xFF,
                 m_cfg.tx_power_dbm);
    m_sleeping = false;
    return true;
}

bool Sx1278Driver::initialize() {
    if (!lock()) return false;
    bool ok = applyChipConfig();
    m_ready = ok;
    unlock();
    return ok;
}

bool Sx1278Driver::setConfig(const RadioConfig& cfg) {
    if (!lock()) return false;
    m_cfg = cfg;
    bool ok = m_ready ? applyChipConfig() : true;
    unlock();
    return ok;
}

bool Sx1278Driver::getConfig(RadioConfig& cfg) const {
    cfg = m_cfg;
    return true;
}

bool Sx1278Driver::sendPacket(const uint8_t* data, size_t len) {
    if (!data || len == 0 || len > kMaxRadioFrame || !m_ready) return false;
    if (!lock()) return false;

    if (m_sleeping) {
        LoRa.idle();
        m_sleeping = false;
    }

    LoRa.beginPacket();
    size_t written = LoRa.write(data, len);
    bool ok = LoRa.endPacket() && written == len;

    if (ok) {
        Logger::debug("SX1278", "TX %u bytes", static_cast<unsigned>(len));
        Logger::dumpHex(LogLevel::Debug, "SX1278", data, len);
    } else {
        Logger::error("SX1278", "TX failed written=%u", static_cast<unsigned>(written));
    }
    unlock();
    return ok;
}

bool Sx1278Driver::receivePacket(RadioPacket& out, uint32_t timeout_ms) {
    if (!m_ready) return false;

    const uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        if (!lock(5)) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        int packetSize = LoRa.parsePacket();
        if (packetSize > 0) {
            out.length = 0;
            while (LoRa.available() && out.length < kMaxRadioFrame) {
                out.data[out.length++] = static_cast<uint8_t>(LoRa.read());
            }
            out.meta.rssi = LoRa.packetRssi();
            out.meta.snr = LoRa.packetSnr();
            out.meta.rx_timestamp_ms = millis();
            m_last_rssi = out.meta.rssi;
            unlock();
            Logger::debug("SX1278", "RX %u bytes RSSI %d SNR %d",
                            out.length, out.meta.rssi, out.meta.snr);
            Logger::dumpHex(LogLevel::Debug, "SX1278", out.data.data(), out.length);
            return true;
        }
        unlock();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    return false;
}

bool Sx1278Driver::sleep() {
    if (!lock()) return false;
    LoRa.sleep();
    m_sleeping = true;
    Logger::debug("SX1278", "sleep");
    unlock();
    return true;
}

bool Sx1278Driver::wake() {
    if (!lock()) return false;
    if (m_sleeping) {
        bool ok = applyChipConfig();
        m_ready = ok;
        unlock();
        return ok;
    }
    unlock();
    return true;
}

int16_t Sx1278Driver::readRSSI() {
    return m_last_rssi;
}

bool Sx1278Driver::isChannelFree() {
    if (!lock(5)) return true;
    bool free = LoRa.parsePacket() == 0;
    unlock();
    return free;
}

} // namespace loralink::radio

#endif // HAS_SX1278
