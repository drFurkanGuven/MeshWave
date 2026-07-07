#pragma once

/**
 * @file sx1278_driver.h
 * @brief SX1278 SPI radio driver (T3 V1.6.1 Gateway).
 */

#include "../radio/i_radio_driver.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace loralink::radio {

/**
 * @class Sx1278Driver
 * @brief IRadioDriver implementation for SX1278 via SPI (LoRa.h).
 */
class Sx1278Driver : public IRadioDriver {
public:
    Sx1278Driver(int cs, int rst, int dio0, int sck, int miso, int mosi);
    ~Sx1278Driver() override;

    Sx1278Driver(const Sx1278Driver&) = delete;
    Sx1278Driver& operator=(const Sx1278Driver&) = delete;

    bool initialize() override;
    bool setConfig(const RadioConfig& cfg) override;
    bool getConfig(RadioConfig& cfg) const override;
    bool sendPacket(const uint8_t* data, size_t len) override;
    bool receivePacket(RadioPacket& out, uint32_t timeout_ms) override;
    bool sleep() override;
    bool wake() override;
    int16_t readRSSI() override;
    bool isChannelFree() override;
    const char* driverName() const override { return "SX1278"; }

private:
    int m_cs, m_rst, m_dio0, m_sck, m_miso, m_mosi;
    RadioConfig m_cfg{};
    bool m_ready{false};
    bool m_sleeping{false};
    int16_t m_last_rssi{0};
    SemaphoreHandle_t m_mutex{nullptr};

    bool applyChipConfig();
    bool lock(uint32_t timeout_ms = 1000);
    void unlock();
};

} // namespace loralink::radio
