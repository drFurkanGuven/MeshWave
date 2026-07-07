#pragma once

/**
 * @file retry_policy.h
 * @brief Exponential backoff retry policy for reliable transport.
 */

#include <cstdint>

namespace loralink::transport {

struct RetryPolicy {
    uint32_t base_timeout_ms{3000};
    uint8_t  max_retries{3};

    uint32_t timeoutForAttempt(uint8_t attempt) const;
    bool shouldRetry(uint8_t attempt) const;
};

} // namespace loralink::transport
