#pragma once

/**
 * @file radio_factory.h
 * @brief Factory for board-specific IRadioDriver instances.
 */

#include "i_radio_driver.h"
#include <memory>

namespace loralink::radio {

enum class BoardType : uint8_t {
    Cardputer,
    GatewayT3,
};

/**
 * @class RadioFactory
 * @brief Creates the correct radio driver for the compile-time board.
 */
class RadioFactory {
public:
    static std::unique_ptr<IRadioDriver> create();
    static BoardType boardType();
};

} // namespace loralink::radio
