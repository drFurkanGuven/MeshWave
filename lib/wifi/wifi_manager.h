#pragma once

/**
 * @file wifi_manager.h
 * @brief WiFi AP lifecycle manager.
 */

#include "ap_config.h"
#include <functional>

namespace loralink::wifi {

enum class WifiState : uint8_t {
    Off,
    Starting,
    Running,
    Error,
};

/**
 * @class WifiManager
 * @brief Manages soft-AP mode with static IP.
 */
class WifiManager {
public:
    using StateCallback = std::function<void(WifiState)>;

    bool begin(const ApConfig& cfg);
    void stop();
    bool isRunning() const { return m_state == WifiState::Running; }
    WifiState state() const { return m_state; }

    void onStateChange(StateCallback cb) { m_callback = cb; }

private:
    ApConfig m_config{};
    WifiState m_state{WifiState::Off};
    StateCallback m_callback;
};

} // namespace loralink::wifi
