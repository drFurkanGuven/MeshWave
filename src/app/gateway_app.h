#pragma once

/**
 * @file gateway_app.h
 * @brief T3 Gateway: WiFi AP + serial chat bridge.
 */

namespace loralink::app {

bool startGateway();
void gatewayLoop();

} // namespace loralink::app
