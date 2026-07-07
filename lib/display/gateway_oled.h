#pragma once

/**
 * @file gateway_oled.h
 * @brief Minimal OLED status display for T3 gateway.
 */

#include "display_renderer.h"

namespace loralink::display {

class GatewayOled : public IDisplayRenderer {
public:
    bool begin() override;
    void renderStatusBar(const StatusBarData& data) override;
    void renderChat(const char* const*, size_t) override {}
    void renderInputLine(const char*, bool) override {}
    void flush() override;
};

} // namespace loralink::display
