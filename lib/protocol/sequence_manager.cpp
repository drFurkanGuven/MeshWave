/**
 * @file sequence_manager.cpp
 */
#include "sequence_manager.h"

namespace loralink::protocol {

SequenceManager::SequenceManager(size_t dedup_window) : m_window(dedup_window) {}

SequenceNum SequenceManager::nextSequence() {
    return m_tx_seq++;
}

bool SequenceManager::isDuplicate(DeviceId source, SequenceNum seq) {
    for (auto& e : m_seen) {
        if (e.used && e.id == source && e.seq == seq) return true;
    }
    return false;
}

void SequenceManager::markReceived(DeviceId source, SequenceNum seq) {
    for (auto& e : m_seen) {
        if (!e.used) {
            e = {source, seq, true};
            return;
        }
    }
    m_seen[seq % m_seen.size()] = {source, seq, true};
}

void SequenceManager::reset() {
    m_tx_seq = 0;
    for (auto& e : m_seen) e.used = false;
}

} // namespace loralink::protocol
