#include "reassembler.h"
#include <cstring>

namespace loralink::fragmentation {

bool Reassembler::ingest(const protocol::Packet& frag) {
    if (!frag.isFragment()) return false;
    uint16_t fid = static_cast<uint16_t>(frag.payload[0] << 8 | frag.payload[1]);
    Key k{frag.source_id, fid};
    auto& a = m_assemblies[k];
    size_t plen = frag.payload_len > 2 ? frag.payload_len - 2 : 0;
    if (a.len + plen <= a.buffer.size()) {
        std::memcpy(a.buffer.data() + a.len, frag.payload.data() + 2, plen);
        a.len += plen;
    }
    if (frag.isLastFragment()) a.complete = true;
    return true;
}

bool Reassembler::isComplete(DeviceId source, uint16_t frag_id) const {
    Key k{source, frag_id};
    auto it = m_assemblies.find(k);
    return it != m_assemblies.end() && it->second.complete;
}

bool Reassembler::takePayload(DeviceId source, uint16_t frag_id,
                              uint8_t* out, size_t out_cap, size_t& out_len) {
    Key k{source, frag_id};
    auto it = m_assemblies.find(k);
    if (it == m_assemblies.end() || !it->second.complete) return false;
    out_len = it->second.len;
    if (out_len > out_cap) return false;
    std::memcpy(out, it->second.buffer.data(), out_len);
    m_assemblies.erase(it);
    return true;
}

void Reassembler::purgeStale(uint32_t) {}

} // namespace loralink::fragmentation
