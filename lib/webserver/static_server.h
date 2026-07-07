#pragma once

/**
 * @file static_server.h
 * @brief HTTP static file server for gateway web UI.
 */

#include <cstdint>

namespace loralink::webserver {

class StaticServer {
public:
    bool begin(uint16_t port = 80);
    void handleClient();
    void stop();
};

} // namespace loralink::webserver
