#pragma once

/**
 * @file fragmenter.h
 * @brief Split large payloads into protocol-sized fragments.
 */

#include "../protocol/packet.h"
#include "../common/types.h"
#include <cstddef>
#include <vector>

namespace loralink::fragmentation {

struct Fragment {
    protocol::Packet packet{};
};

/**
 * @class Fragmenter
 * @brief Splits data into FRAGMENT/LAST_FRAG flagged packets.
 */
class Fragmenter {
public:
    static constexpr size_t kChunkSize = kMaxPayloadSize - 4; // frag header

    std::vector<Fragment> split(const uint8_t* data, size_t len, DeviceId dest,
                                protocol::SequenceNum base_seq) const;
};

} // namespace loralink::fragmentation
