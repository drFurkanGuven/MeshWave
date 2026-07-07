#pragma once

/**
 * @file sequence_manager.h
 * @brief TX sequence counter and RX deduplication window.
 */

#include "../common/types.h"
#include <cstdint>
#include <array>

namespace loralink::protocol {

/**
 * @class SequenceManager
 * @brief Manages outbound sequence numbers and duplicate detection.
 */
class SequenceManager {
public:
    explicit SequenceManager(size_t dedup_window = 64);

    SequenceNum nextSequence();
    bool isDuplicate(DeviceId source, SequenceNum seq);
    void markReceived(DeviceId source, SequenceNum seq);
    void reset();

private:
    SequenceNum m_tx_seq{0};
    size_t m_window;
    struct Entry { DeviceId id; SequenceNum seq; bool used; };
    std::array<Entry, 64> m_seen{};
};

} // namespace loralink::protocol
