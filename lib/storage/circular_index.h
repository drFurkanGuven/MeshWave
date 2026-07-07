#pragma once

/**
 * @file circular_index.h
 * @brief Circular index for 1000-message flash store.
 */

#include "../common/types.h"
#include <cstdint>

namespace loralink::storage {

struct CircularIndex {
    uint32_t head{0};
    uint32_t count{0};
    uint32_t next_id{1};

    static constexpr uint32_t kCapacity = kMaxMessageStore;

    uint32_t pushIndex() const;
    void advance();
};

} // namespace loralink::storage
