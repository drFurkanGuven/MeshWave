#pragma once

/**
 * @file packet_builder.h
 * @brief Factory helpers for common packet types.
 */

#include "packet.h"
#include "../common/types.h"

namespace loralink::protocol {

Packet makePing(DeviceId src, DeviceId dest, SequenceNum seq);
Packet makePong(const Packet& ping);
Packet makeAck(const Packet& orig, DeviceId src);
Packet makeHello(DeviceId src, DeviceId dest, SequenceNum seq);
Packet makeMessage(DeviceId src, DeviceId dest, SequenceNum seq,
                   const uint8_t* data, size_t len, bool require_ack = true);

} // namespace loralink::protocol
