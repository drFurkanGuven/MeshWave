/**
 * @file ack_manager.cpp
 */
#include "ack_manager.h"

namespace loralink::transport {

AckManager::AckManager(const RetryPolicy& policy) : m_policy(policy) {}

bool AckManager::addPending(const protocol::Packet& pkt, uint32_t now_ms) {
    for (auto& p : m_pending) {
        if (!p.active) {
            p = {pkt, 0, now_ms, true};
            return true;
        }
    }
    return false;
}

bool AckManager::onAck(SequenceNum seq, DeviceId from) {
    for (auto& p : m_pending) {
        if (p.active && p.packet.sequence == seq && p.packet.dest_id == from) {
            p.active = false;
            return true;
        }
    }
    return false;
}

PendingTx* AckManager::nextExpired(uint32_t now_ms) {
    for (auto& p : m_pending) {
        if (!p.active) continue;
        if (now_ms - p.sent_at_ms >= m_policy.timeoutForAttempt(p.attempts)) {
            return &p;
        }
    }
    return nullptr;
}

void AckManager::clear() {
    for (auto& p : m_pending) p.active = false;
}

size_t AckManager::pendingCount() const {
    size_t n = 0;
    for (const auto& p : m_pending) if (p.active) n++;
    return n;
}

} // namespace loralink::transport
