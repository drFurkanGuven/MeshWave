#include "circular_index.h"

namespace loralink::storage {

uint32_t CircularIndex::pushIndex() const {
    return (head + count) % kCapacity;
}

void CircularIndex::advance() {
    if (count < kCapacity) {
        count++;
    } else {
        head = (head + 1) % kCapacity;
    }
    next_id++;
}

} // namespace loralink::storage
