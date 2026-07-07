/**
 * @file radio_factory.cpp
 */
#include "radio_factory.h"
#include <memory>

#if defined(HAS_E220) && HAS_E220
#include "../e220/e220_driver.h"
#include "board.h"
#endif

#if defined(HAS_SX1278) && HAS_SX1278
#include "../sx1278/sx1278_driver.h"
#include "board.h"
#endif

namespace loralink::radio {

BoardType RadioFactory::boardType() {
#if defined(BOARD_CARDPUTER)
    return BoardType::Cardputer;
#elif defined(BOARD_T3_V161)
    return BoardType::GatewayT3;
#else
    return BoardType::Cardputer;
#endif
}

std::unique_ptr<IRadioDriver> RadioFactory::create() {
#if defined(HAS_E220) && HAS_E220
    return std::make_unique<E220Driver>(E220_UART_TX, E220_UART_RX, E220_UART_BAUD);
#elif defined(HAS_SX1278) && HAS_SX1278
    return std::make_unique<Sx1278Driver>(LORA_CS, LORA_RST, LORA_DIO0,
                                          LORA_SCK, LORA_MISO, LORA_MOSI);
#else
    return nullptr;
#endif
}

} // namespace loralink::radio
