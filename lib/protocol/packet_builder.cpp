#include "packet_builder.h"
#include <Arduino.h>
#include <cstring>

namespace loralink::protocol {

static void fillBase(Packet& p, PacketType type, DeviceId src, DeviceId dest, SequenceNum seq) {
    p.version = kVersion;
    p.type = type;
    p.flags = 0;
    p.sequence = seq;
    p.source_id = src;
    p.dest_id = dest;
    p.hop_count = 0;
    p.ttl = 8;
    p.timestamp = static_cast<Timestamp>(millis() / 1000UL);
    p.payload_len = 0;
}

Packet makePing(DeviceId src, DeviceId dest, SequenceNum seq) {
    Packet p{};
    fillBase(p, PacketType::Ping, src, dest, seq);
    return p;
}

Packet makePong(const Packet& ping) {
    Packet p{};
    fillBase(p, PacketType::Pong, ping.dest_id, ping.source_id, ping.sequence);
    p.payload[0] = static_cast<uint8_t>(ping.sequence >> 8);
    p.payload[1] = static_cast<uint8_t>(ping.sequence);
    p.payload_len = 2;
    return p;
}

Packet makeAck(const Packet& orig, DeviceId src) {
    Packet p{};
    fillBase(p, PacketType::Ack, src, orig.source_id, orig.sequence);
    return p;
}

Packet makeHello(DeviceId src, DeviceId dest, SequenceNum seq) {
    Packet p{};
    fillBase(p, PacketType::Hello, src, dest, seq);
    const char* tag = "LoRaLink";
    std::memcpy(p.payload.data(), tag, 8);
    p.payload_len = 8;
    return p;
}

Packet makeMessage(DeviceId src, DeviceId dest, SequenceNum seq,
                   const uint8_t* data, size_t len, bool require_ack) {
    Packet p{};
    fillBase(p, PacketType::Message, src, dest, seq);
    if (require_ack) p.flags |= FlagAckReq;
    if (data && len > 0) {
        if (len > kMaxPayloadSize) len = kMaxPayloadSize;
        std::memcpy(p.payload.data(), data, len);
        p.payload_len = static_cast<uint16_t>(len);
    }
    return p;
}

} // namespace loralink::protocol
