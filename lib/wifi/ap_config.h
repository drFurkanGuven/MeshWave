#pragma once

/**
 * @file ap_config.h
 * @brief WiFi Access Point configuration.
 */

#include <cstdint>

namespace loralink::wifi {

struct ApConfig {
    char     ssid[33]{};
    char     password[65]{};
    char     ip[16]{"192.168.4.1"};
    char     gateway[16]{"192.168.4.1"};
    char     subnet[16]{"255.255.255.0"};
    uint8_t  channel{6};
    uint8_t  max_clients{4};
    bool     hidden{false};
};

} // namespace loralink::wifi
