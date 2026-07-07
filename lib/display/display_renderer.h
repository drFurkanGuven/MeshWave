#pragma once

/**
 * @file display_renderer.h
 * @brief Abstract display renderer with dirty-region support.
 */

#include <cstdint>
#include <cstddef>

namespace loralink::display {

struct StatusBarData {
    bool    lora_ok{false};
    int16_t rssi{0};
    int8_t  snr{0};
    uint8_t battery_pct{0};
    uint8_t channel{0};
    size_t  unread{0};
    char    clock_str[6]{"--:--"};
};

/**
 * @class IDisplayRenderer
 * @brief Platform-agnostic display interface.
 */
class IDisplayRenderer {
public:
    virtual ~IDisplayRenderer() = default;
    virtual bool begin() = 0;
    virtual void renderStatusBar(const StatusBarData& data) = 0;
    virtual void renderChat(const char* const* lines, size_t count) = 0;
    virtual void renderInputLine(const char* buffer, bool cursor_on) = 0;
    virtual void flush() = 0;
};

} // namespace loralink::display
