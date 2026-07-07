#include "wifi_manager.h"
#include "../common/logger.h"
#include <WiFi.h>
#include <cstring>

namespace loralink::wifi {

using loralink::Logger;

bool WifiManager::begin(const ApConfig& cfg) {
    m_config = cfg;
    m_state = WifiState::Starting;

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    const bool ok = WiFi.softAP(cfg.ssid, cfg.password[0] ? cfg.password : nullptr,
                                cfg.channel, cfg.hidden, cfg.max_clients);
    if (!ok) {
        m_state = WifiState::Error;
        Logger::error("WIFI", "softAP failed");
        if (m_callback) m_callback(m_state);
        return false;
    }

    m_state = WifiState::Running;
    Logger::info("WIFI", "AP %s @ %s", cfg.ssid, WiFi.softAPIP().toString().c_str());
    if (m_callback) m_callback(m_state);
    return true;
}

void WifiManager::stop() {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    m_state = WifiState::Off;
    if (m_callback) m_callback(m_state);
}

} // namespace loralink::wifi
