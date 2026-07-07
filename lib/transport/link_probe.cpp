#include "link_probe.h"
#include "../common/logger.h"

namespace loralink::transport {

using loralink::Logger;

LinkProbe::LinkProbe(ReliableTransport& transport, DeviceId peer_id)
    : m_transport(transport), m_peer(peer_id) {}

void LinkProbe::poll(uint32_t now_ms) {
    if (now_ms - m_last_ping_ms < m_interval_ms) return;
    m_last_ping_ms = now_ms;

    if (m_transport.sendPing(m_peer)) {
        Logger::debug("PROBE", "PING -> %04X", m_peer);
    }

    const auto& s = m_transport.stats();
    if (s.link_up && (now_ms - s.last_peer_ms) < 15000) {
        Logger::info("PROBE", "link UP pongs=%u rssi path active", s.pongs_rx);
    }
}

} // namespace loralink::transport
