#pragma once

/**
 * @file link_probe.h
 * @brief Periodic PING/HELLO for link validation (Phase 6 test).
 */

#include "reliable_transport.h"

namespace loralink::transport {

/**
 * @class LinkProbe
 * @brief Sends periodic PING to peer; logs link status from transport stats.
 */
class LinkProbe {
public:
    LinkProbe(ReliableTransport& transport, DeviceId peer_id);

    void poll(uint32_t now_ms);
    void setIntervalMs(uint32_t ms) { m_interval_ms = ms; }

private:
    ReliableTransport& m_transport;
    DeviceId m_peer;
    uint32_t m_interval_ms{5000};
    uint32_t m_last_ping_ms{0};
    bool m_hello_sent{false};
};

} // namespace loralink::transport
