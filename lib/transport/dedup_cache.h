#pragma once

/**
 * @file dedup_cache.h
 * @brief Duplicate packet detection cache.
 */

#include "../common/types.h"
#include "../protocol/sequence_manager.h"

namespace loralink::transport {

using DedupCache = loralink::protocol::SequenceManager;

} // namespace loralink::transport
