#include "router.h"
#include "../protocol/packet_types.h"

namespace loralink::routing {

Router::Router(DeviceId local_id) : m_local_id(local_id) {}

bool Router::isForUs(const protocol::Packet& pkt) const {
    return pkt.dest_id == m_local_id || pkt.dest_id == kBroadcastId;
}

void Router::prepareOutbound(protocol::Packet& pkt, DeviceId dest) const {
    pkt.source_id = m_local_id;
    pkt.dest_id = dest;
    pkt.hop_count = 0;
    pkt.ttl = 8;
    if (dest == kBroadcastId) pkt.flags |= protocol::FlagBroadcast;
}

} // namespace loralink::routing
