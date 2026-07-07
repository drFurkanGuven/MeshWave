#include "gateway_oled.h"

#if defined(BOARD_T3_V161) || defined(ROLE_GATEWAY)

namespace loralink::display {

bool GatewayOled::begin() { return true; }
void GatewayOled::renderStatusBar(const StatusBarData&) {}
void GatewayOled::flush() {}

} // namespace loralink::display

#endif
