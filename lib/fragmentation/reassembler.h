#pragma once

/**
 * @file reassembler.h
 * @brief Reassemble fragmented packets into complete payload.
 */

#include "../protocol/packet.h"
#include <cstdint>
#include <array>
#include <map>

namespace loralink::fragmentation {

/**
 * @class Reassembler
 * @brief Collects fragments until LAST_FRAG received.
 */
class Reassembler {
public:
    bool ingest(const protocol::Packet& frag);
    bool isComplete(DeviceId source, uint16_t frag_id) const;
    bool takePayload(DeviceId source, uint16_t frag_id,
                     uint8_t* out, size_t out_cap, size_t& out_len);
    void purgeStale(uint32_t now_ms);

private:
    struct Assembly {
        std::array<uint8_t, kMaxChatText> buffer{};
        size_t len{0};
        uint32_t updated_ms{0};
        bool complete{false};
    };
    struct Key { DeviceId src; uint16_t id; bool operator<(const Key& o) const {
        return src < o.src || (src == o.src && id < o.id);
    }};
    std::map<Key, Assembly> m_assemblies;
};

} // namespace loralink::fragmentation
