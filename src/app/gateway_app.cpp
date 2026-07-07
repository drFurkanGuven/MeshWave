/**
 * @file gateway_app.cpp
 * @brief Gateway: WiFi AP, web UI, WebSocket chat, serial fallback.
 */
#include "gateway_app.h"
#include "app_core.h"
#include "board.h"
#include "../../lib/wifi/wifi_manager.h"
#include "../../lib/wifi/ap_config.h"
#include "../../lib/chat/chat_service.h"
#include "../../lib/drivers/radio/radio_factory.h"
#include "../../lib/webserver/gateway_web.h"
#include "../../lib/common/logger.h"
#include <Arduino.h>
#include <cstring>
#include <memory>

namespace loralink::app {

using loralink::Logger;
using loralink::kCardputerId;
using loralink::radio::RadioFactory;
using loralink::wifi::ApConfig;
using loralink::wifi::WifiManager;
using loralink::webserver::GatewayWeb;
using loralink::webserver::GatewayWebContext;

static WifiManager s_wifi;
static GatewayWeb s_web;
static bool s_started{false};

bool startGateway() {
    auto radio = RadioFactory::create();
    if (!radio) return false;

    AppOptions opts{};
    opts.role = DeviceRole::Gateway;
    opts.peer_id = kCardputerId;
    opts.send_probes = true;
    opts.probe_interval_ms = 5000;

    s_started = start(std::move(radio), opts);
    if (!s_started) return false;

    const auto& cfg = config();
    ApConfig ap{};
    std::strncpy(ap.ssid, cfg.wifi_ssid, sizeof(ap.ssid) - 1);
    std::strncpy(ap.password, cfg.wifi_password, sizeof(ap.password) - 1);
    if (!s_wifi.begin(ap)) {
        Logger::warning("GW", "WiFi AP start failed — continuing LoRa only");
    } else {
        Logger::info("GW", "AP: %s IP %s", ap.ssid, ap.ip);
    }

#if HAS_WEB_SERVER
    GatewayWebContext web_ctx{};
    web_ctx.chat = chat();
    web_ctx.transport = transport();
    web_ctx.radio = loralink::app::radio();
    web_ctx.config = &config();
    if (!s_web.begin(web_ctx)) {
        Logger::warning("GW", "Web server start failed");
    }
#endif

    Serial.println("[LoRaLink] Serial chat: type message + Enter");
    Serial.println("[LoRaLink] Web UI: http://192.168.4.1");
    return true;
}

void gatewayLoop() {
    if (!s_started) {
        vTaskDelay(pdMS_TO_TICKS(200));
        return;
    }

    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() && chat()) {
            chat()->sendText(line.c_str(), kCardputerId);
        }
    }

#if HAS_WEB_SERVER
    s_web.poll(millis());
#endif

    vTaskDelay(pdMS_TO_TICKS(20));
}

} // namespace loralink::app
