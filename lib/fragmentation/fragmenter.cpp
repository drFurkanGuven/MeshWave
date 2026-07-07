#include "fragmenter.h"
#include "../protocol/packet_types.h"
#include <cstring>

namespace loralink::fragmentation {

std::vector<Fragment> Fragmenter::split(const uint8_t* data, size_t len, DeviceId dest,
                                        protocol::SequenceNum base_seq) const {
    std::vector<Fragment> out;
    if (!data || len == 0) return out;
    size_t offset = 0;
    uint16_t frag_id = 0;
    while (offset < len) {
        Fragment f{};
        size_t chunk = len - offset;
        if (chunk > kChunkSize) chunk = kChunkSize;
        f.packet.type = protocol::PacketType::Message;
        f.packet.dest_id = dest;
        f.packet.sequence = base_seq + static_cast<SequenceNum>(out.size());
        f.packet.flags |= protocol::FlagFragment;
        if (offset + chunk >= len) f.packet.flags |= protocol::FlagLastFrag;
        f.packet.payload[0] = static_cast<uint8_t>(frag_id >> 8);
        f.packet.payload[1] = static_cast<uint8_t>(frag_id);
        std::memcpy(f.packet.payload.data() + 2, data + offset, chunk);
        f.packet.payload_len = static_cast<uint16_t>(chunk + 2);
        out.push_back(f);
        offset += chunk;
        frag_id++;
    }
    return out;
}

} // namespace loralink::fragmentation
