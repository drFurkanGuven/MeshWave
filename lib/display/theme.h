#pragma once

/**
 * @file theme.h
 * @brief Shared dark theme color constants.
 */

#include <cstdint>

namespace loralink::display {

struct Theme {
    static constexpr uint16_t bg       = 0x0000; // #0D1117 approx
    static constexpr uint16_t surface    = 0x1082;
    static constexpr uint16_t text       = 0xFFFF;
    static constexpr uint16_t accent     = 0x4F9F;
    static constexpr uint16_t success    = 0x3666;
    static constexpr uint16_t warning    = 0xFD20;
    static constexpr uint16_t muted      = 0x8410;
};

} // namespace loralink::display
