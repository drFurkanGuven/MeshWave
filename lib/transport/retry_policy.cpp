#include "retry_policy.h"

namespace loralink::transport {

uint32_t RetryPolicy::timeoutForAttempt(uint8_t attempt) const {
    uint32_t mult = 1u << (attempt > 3 ? 3 : attempt);
    return base_timeout_ms * mult;
}

bool RetryPolicy::shouldRetry(uint8_t attempt) const {
    return attempt < max_retries;
}

} // namespace loralink::transport
