/**
 * @file packet_codec.cpp
 * @brief LoRaLink wire packet encoder/decoder.
 */
#include "packet_codec.h"
#include "crc16.h"
#include "wire_endian.h"
#include "../common/errors.h"
#include "../common/logger.h"
#include <cstring>

namespace loralink::protocol {

using loralink::Error;
using loralink::Logger;
using wire::readU16LE;
using wire::readU32LE;
using wire::writeU16LE;
using wire::writeU32LE;

Result<size_t> PacketCodec::encode(const Packet& pkt, uint8_t* out, size_t out_cap) {
    if (!out) {
        return Result<size_t>::failure(static_cast<int>(Error::InvalidArgument));
    }
    if (pkt.payload_len > kMaxPayloadSize) {
        return Result<size_t>::failure(static_cast<int>(Error::InvalidArgument));
    }

    const size_t total = kHeaderSize + pkt.payload_len + kCrcSize;
    if (out_cap < total) {
        return Result<size_t>::failure(static_cast<int>(Error::BufferTooSmall));
    }

    writeU16LE(out + 0, kMagic);
    out[2] = pkt.version;
    out[3] = static_cast<uint8_t>(pkt.type);
    out[4] = pkt.flags;
    writeU16LE(out + 5, pkt.sequence);
    writeU16LE(out + 7, pkt.source_id);
    writeU16LE(out + 9, pkt.dest_id);
    out[11] = pkt.hop_count;
    out[12] = pkt.ttl;
    writeU32LE(out + 13, pkt.timestamp);
    writeU16LE(out + 17, pkt.payload_len);

    if (pkt.payload_len > 0) {
        std::memcpy(out + kHeaderSize, pkt.payload.data(), pkt.payload_len);
    }

    const uint16_t crc = crc16(out, kHeaderSize + pkt.payload_len);
    writeU16LE(out + kHeaderSize + pkt.payload_len, crc);

    return Result<size_t>::success(total);
}

Result<Packet> PacketCodec::decode(const uint8_t* data, size_t len) {
    if (!data || len < kHeaderSize + kCrcSize) {
        return Result<Packet>::failure(static_cast<int>(Error::DecodeFailed));
    }

    if (readU16LE(data) != kMagic) {
        return Result<Packet>::failure(static_cast<int>(Error::DecodeFailed));
    }

    Packet pkt{};
    pkt.version = data[2];
    if (pkt.version != kVersion) {
        return Result<Packet>::failure(static_cast<int>(Error::DecodeFailed));
    }

    pkt.type = static_cast<PacketType>(data[3]);
    pkt.flags = data[4];
    pkt.sequence = readU16LE(data + 5);
    pkt.source_id = readU16LE(data + 7);
    pkt.dest_id = readU16LE(data + 9);
    pkt.hop_count = data[11];
    pkt.ttl = data[12];
    pkt.timestamp = readU32LE(data + 13);
    pkt.payload_len = readU16LE(data + 17);

    if (pkt.payload_len > kMaxPayloadSize) {
        return Result<Packet>::failure(static_cast<int>(Error::DecodeFailed));
    }

    const size_t expected = kHeaderSize + pkt.payload_len + kCrcSize;
    if (len < expected) {
        return Result<Packet>::failure(static_cast<int>(Error::DecodeFailed));
    }

    const uint16_t rx_crc = readU16LE(data + kHeaderSize + pkt.payload_len);
    const uint16_t calc = crc16(data, kHeaderSize + pkt.payload_len);
    if (rx_crc != calc) {
        Logger::warning("PROTO", "CRC mismatch rx=%04X calc=%04X", rx_crc, calc);
        return Result<Packet>::failure(static_cast<int>(Error::CrcMismatch));
    }

    if (pkt.payload_len > 0) {
        std::memcpy(pkt.payload.data(), data + kHeaderSize, pkt.payload_len);
    }

    return Result<Packet>::success(pkt);
}

size_t PacketCodec::wireSize(const Packet& pkt) {
    return kHeaderSize + pkt.payload_len + kCrcSize;
}

} // namespace loralink::protocol
