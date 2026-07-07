#pragma once

/**
 * @file packet.h
 * @brief In-memory representation of a LoRaLink protocol packet.
 */

#include "packet_types.h"
#include "../common/types.h"
#include <cstdint>
#include <array>

namespace loralink::protocol {

struct Packet {
    uint8_t     version{kVersion};
    PacketType  type{PacketType::Message};
    uint8_t     flags{0};
    SequenceNum sequence{0};
    DeviceId    source_id{0};
    DeviceId    dest_id{0};
    uint8_t     hop_count{0};
    uint8_t     ttl{8};
    Timestamp   timestamp{0};
    std::array<uint8_t, kMaxPayloadSize> payload{};
    uint16_t    payload_len{0};

    bool isAckRequired() const { return flags & FlagAckReq; }
    bool isEncrypted() const { return flags & FlagEncrypted; }
    bool isFragment() const { return flags & FlagFragment; }
    bool isLastFragment() const { return flags & FlagLastFrag; }
};

} // namespace loralink::protocol
