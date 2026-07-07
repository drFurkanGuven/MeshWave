#pragma once

/**
 * @file packet_codec.h
 * @brief Encode/decode LoRaLink packets to wire format.
 */

#include "packet.h"
#include "../common/errors.h"
#include "../common/types.h"
#include <cstddef>
#include <cstdint>

namespace loralink::protocol {

/**
 * @class PacketCodec
 * @brief Stateless serializer for LoRaLink wire packets.
 */
class PacketCodec {
public:
    static Result<size_t> encode(const Packet& pkt, uint8_t* out, size_t out_cap);
    static Result<Packet> decode(const uint8_t* data, size_t len);
    static size_t wireSize(const Packet& pkt);
};

} // namespace loralink::protocol
