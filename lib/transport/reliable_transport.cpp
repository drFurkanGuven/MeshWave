/**
 * @file reliable_transport.cpp
 * @brief Reliable transport with ACK, retry, dedup, PING/PONG.
 */
#include <Arduino.h>
#include "reliable_transport.h"
#include "../protocol/packet_builder.h"
#include "../common/logger.h"
#include <cstring>

namespace loralink::transport {

using loralink::Logger;
using namespace loralink::protocol;

ReliableTransport::ReliableTransport(radio::IRadioDriver& radio, const RetryPolicy& policy)
    : m_radio(radio), m_policy(policy), m_ack_mgr(policy) {}

bool ReliableTransport::begin() {
    return m_radio.initialize();
}

void ReliableTransport::setLocalId(DeviceId id) {
    m_local_id = id;
}

bool ReliableTransport::transmitWire(const uint8_t* data, size_t len) {
    if (!data || len == 0 || len > kMaxWireSize) return false;
    return m_radio.sendPacket(data, len);
}

bool ReliableTransport::transmitPacket(const Packet& pkt) {
    uint8_t wire[kMaxWireSize];
    auto enc = PacketCodec::encode(pkt, wire, sizeof(wire));
    if (!enc.ok) {
        Logger::error("TRANS", "encode failed err=%d", enc.error);
        return false;
    }
    if (!transmitWire(wire, enc.value)) {
        Logger::error("TRANS", "radio TX failed");
        return false;
    }
    m_stats.tx_count++;
    Logger::debug("TRANS", "TX type=%u seq=%u -> %04X",
                  static_cast<unsigned>(pkt.type), pkt.sequence, pkt.dest_id);
    return true;
}

bool ReliableTransport::send(Packet pkt, bool require_ack) {
    pkt.source_id = m_local_id;
    if (pkt.timestamp == 0) {
        pkt.timestamp = static_cast<Timestamp>(millis() / 1000UL);
    }
    if (require_ack) {
        pkt.flags |= FlagAckReq;
    }

    if (!transmitPacket(pkt)) return false;

    if (require_ack) {
        if (!m_ack_mgr.addPending(pkt, millis())) {
            Logger::warning("TRANS", "ACK queue full");
        }
    }
    return true;
}

bool ReliableTransport::sendMessage(DeviceId dest, const char* text, size_t len) {
    if (!text || len == 0) return false;
    if (len > kMaxPayloadSize) len = kMaxPayloadSize;
    Packet pkt = makeMessage(m_local_id, dest, m_seq.nextSequence(),
                             reinterpret_cast<const uint8_t*>(text), len, true);
    return send(pkt, true);
}

bool ReliableTransport::sendPing(DeviceId dest) {
    Packet pkt = makePing(m_local_id, dest, m_seq.nextSequence());
    return send(pkt, false);
}

void ReliableTransport::sendAck(const Packet& orig) {
    Packet ack = makeAck(orig, m_local_id);
    transmitPacket(ack);
    m_stats.ack_count++;
}

bool ReliableTransport::isForUs(const Packet& pkt) const {
    return pkt.dest_id == m_local_id || pkt.dest_id == kBroadcastId;
}

void ReliableTransport::handleAutoReply(const Packet& pkt) {
    if (pkt.type == PacketType::Ping) {
        Packet pong = makePong(pkt);
        pong.source_id = m_local_id;
        transmitPacket(pong);
        Logger::info("TRANS", "PONG -> %04X seq=%u", pkt.source_id, pkt.sequence);
        return;
    }
    if (pkt.type == PacketType::Hello) {
        Packet hello = makeHello(m_local_id, pkt.source_id, m_seq.nextSequence());
        transmitPacket(hello);
        return;
    }
}

void ReliableTransport::handleRx(const radio::RadioPacket& raw, uint32_t now_ms) {
    auto dec = PacketCodec::decode(raw.data.data(), raw.length);
    if (!dec.ok) {
        if (dec.error == static_cast<int>(loralink::Error::CrcMismatch)) {
            m_stats.crc_errors++;
        }
        return;
    }

    Packet& pkt = dec.value;
    m_stats.rx_count++;
    m_stats.last_rssi = raw.meta.rssi;
    m_stats.last_snr = raw.meta.snr;

    if (pkt.type == PacketType::Ack) {
        if (m_ack_mgr.onAck(pkt.sequence, pkt.source_id)) {
            Logger::debug("TRANS", "ACK seq=%u from %04X", pkt.sequence, pkt.source_id);
        }
        return;
    }

    if (!isForUs(pkt)) return;

    if (pkt.type != PacketType::Ping && m_seq.isDuplicate(pkt.source_id, pkt.sequence)) {
        m_stats.duplicates++;
        Logger::debug("TRANS", "duplicate seq=%u from %04X", pkt.sequence, pkt.source_id);
        return;
    }

    if (pkt.type == PacketType::Ping) {
        m_stats.pings_rx++;
        m_stats.link_up = true;
        m_stats.last_peer_ms = now_ms;
        handleAutoReply(pkt);
        m_seq.markReceived(pkt.source_id, pkt.sequence);
        return;
    }

    if (pkt.type == PacketType::Pong) {
        m_stats.pongs_rx++;
        m_stats.link_up = true;
        m_stats.last_peer_ms = now_ms;
        Logger::info("TRANS", "PONG from %04X seq=%u RSSI %d",
                     pkt.source_id, pkt.sequence, raw.meta.rssi);
        m_seq.markReceived(pkt.source_id, pkt.sequence);
        return;
    }

    m_seq.markReceived(pkt.source_id, pkt.sequence);
    m_stats.link_up = true;
    m_stats.last_peer_ms = now_ms;

    if (pkt.isAckRequired()) {
        sendAck(pkt);
    }

    if (m_inbound) {
        m_inbound->send(pkt, 10);
    }
}

void ReliableTransport::processRetries(uint32_t now_ms) {
    for (;;) {
        PendingTx* pending = m_ack_mgr.nextExpired(now_ms);
        if (!pending) break;

        if (!m_policy.shouldRetry(pending->attempts)) {
            Logger::warning("TRANS", "ACK timeout seq=%u", pending->packet.sequence);
            pending->active = false;
            continue;
        }

        pending->attempts++;
        pending->sent_at_ms = now_ms;
        m_stats.retries++;
        Logger::warning("TRANS", "retry seq=%u attempt=%u",
                        pending->packet.sequence, pending->attempts);
        transmitPacket(pending->packet);
    }
}

void ReliableTransport::poll(uint32_t now_ms) {
    processRetries(now_ms);

    radio::RadioPacket raw;
    while (m_radio.receivePacket(raw, 2)) {
        handleRx(raw, now_ms);
    }
}

void ReliableTransport::setInboundQueue(QueueHandle<protocol::Packet>* q) {
    m_inbound = q;
}

} // namespace loralink::transport
