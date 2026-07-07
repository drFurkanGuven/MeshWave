#pragma once

/**
 * @file ack_manager.h
 * @brief Pending ACK tracking for reliable transport.
 */

#include "../protocol/packet.h"
#include "../common/types.h"
#include "retry_policy.h"
#include <cstdint>
#include <array>

namespace loralink::transport {

struct PendingTx {
    protocol::Packet packet{};
    uint8_t          attempts{0};
    uint32_t         sent_at_ms{0};
    bool             active{false};
};

/**
 * @class AckManager
 * @brief Tracks outbound packets awaiting ACK.
 */
class AckManager {
public:
    explicit AckManager(const RetryPolicy& policy);

    bool addPending(const protocol::Packet& pkt, uint32_t now_ms);
    bool onAck(SequenceNum seq, DeviceId from);
    PendingTx* nextExpired(uint32_t now_ms);
    void clear();
    size_t pendingCount() const;

private:
    RetryPolicy m_policy;
    static constexpr size_t kMaxPending = 8;
    std::array<PendingTx, kMaxPending> m_pending{};
};

} // namespace loralink::transport
