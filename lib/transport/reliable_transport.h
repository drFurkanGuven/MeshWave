#pragma once

/**
 * @file reliable_transport.h
 * @brief ACK-based reliable delivery over IRadioDriver.
 */

#include "../drivers/radio/i_radio_driver.h"
#include "../protocol/packet_codec.h"
#include "../protocol/sequence_manager.h"
#include "ack_manager.h"
#include "retry_policy.h"
#include "../common/queues.h"

namespace loralink::transport {

struct TransportStats {
    uint32_t tx_count{0};
    uint32_t rx_count{0};
    uint32_t ack_count{0};
    uint32_t retries{0};
    uint32_t crc_errors{0};
    uint32_t duplicates{0};
    uint32_t pings_rx{0};
    uint32_t pongs_rx{0};
    bool     link_up{false};
    uint32_t last_peer_ms{0};
    int16_t  last_rssi{0};
    int8_t   last_snr{0};
};

/**
 * @class ReliableTransport
 * @brief Sends/receives protocol packets with ACK, retry, and dedup.
 */
class ReliableTransport {
public:
    ReliableTransport(radio::IRadioDriver& radio, const RetryPolicy& policy);

    bool begin();
    void setLocalId(DeviceId id);
    void poll(uint32_t now_ms);

    bool send(protocol::Packet pkt, bool require_ack);
    bool sendMessage(DeviceId dest, const char* text, size_t len);
    bool sendPing(DeviceId dest);

    void setInboundQueue(QueueHandle<protocol::Packet>* q);
    const TransportStats& stats() const { return m_stats; }

private:
    bool transmitPacket(const protocol::Packet& pkt);
    bool transmitWire(const uint8_t* data, size_t len);
    void handleRx(const radio::RadioPacket& raw, uint32_t now_ms);
    void processRetries(uint32_t now_ms);
    void sendAck(const protocol::Packet& orig);
    void handleAutoReply(const protocol::Packet& pkt);
    bool isForUs(const protocol::Packet& pkt) const;

    radio::IRadioDriver& m_radio;
    RetryPolicy m_policy;
    AckManager m_ack_mgr;
    protocol::SequenceManager m_seq;
    QueueHandle<protocol::Packet>* m_inbound{nullptr};
    TransportStats m_stats{};
    DeviceId m_local_id{kCardputerId};
};

} // namespace loralink::transport
