#pragma once

/**
 * @file e220_driver.h
 * @brief EBYTE E220-400T22S UART radio driver (Cardputer).
 */

#include "../radio/i_radio_driver.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace loralink::radio {

/**
 * @class E220Driver
 * @brief IRadioDriver implementation for E220-400T22S via UART.
 *
 * Requires DIP switches M0=OFF M1=OFF for normal TX/RX.
 * For setConfig(), briefly set M0=ON M1=ON (config mode).
 */
class E220Driver : public IRadioDriver {
public:
    E220Driver(int tx_pin, int rx_pin, uint32_t uart_baud = 9600);
    ~E220Driver() override;

    E220Driver(const E220Driver&) = delete;
    E220Driver& operator=(const E220Driver&) = delete;

    bool initialize() override;
    bool setConfig(const RadioConfig& cfg) override;
    bool getConfig(RadioConfig& cfg) const override;
    bool sendPacket(const uint8_t* data, size_t len) override;
    bool receivePacket(RadioPacket& out, uint32_t timeout_ms) override;
    bool sleep() override;
    bool wake() override;
    int16_t readRSSI() override;
    bool isChannelFree() override;
    const char* driverName() const override { return "E220-400T22S"; }

private:
    struct Impl;
    Impl* m_impl;

    int m_tx;
    int m_rx;
    uint32_t m_baud;
    RadioConfig m_cfg{};
    bool m_ready{false};
    bool m_configured{false};
    int16_t m_last_rssi{0};
    SemaphoreHandle_t m_mutex{nullptr};

    bool applyModuleConfig();
    bool lock(uint32_t timeout_ms = 1000);
    void unlock();
};

} // namespace loralink::radio
