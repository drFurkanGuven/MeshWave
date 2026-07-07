#pragma once

/**
 * @file i_radio_driver.h
 * @brief Abstract radio driver interface — sole hardware boundary for LoRa.
 */

#include "radio_config.h"
#include "radio_packet.h"
#include "../../common/errors.h"
#include <cstddef>
#include <cstdint>

namespace loralink::radio {

/**
 * @class IRadioDriver
 * @brief Uniform API for E220 (UART) and SX1278 (SPI) backends.
 */
class IRadioDriver {
public:
    virtual ~IRadioDriver() = default;

    virtual bool initialize() = 0;
    virtual bool setConfig(const RadioConfig& cfg) = 0;
    virtual bool getConfig(RadioConfig& cfg) const = 0;

    virtual bool sendPacket(const uint8_t* data, size_t len) = 0;
    virtual bool receivePacket(RadioPacket& out, uint32_t timeout_ms) = 0;

    virtual bool sleep() = 0;
    virtual bool wake() = 0;

    virtual int16_t readRSSI() = 0;
    virtual bool isChannelFree() = 0;

    virtual const char* driverName() const = 0;
};

} // namespace loralink::radio
