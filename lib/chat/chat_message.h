#pragma once

/**
 * @file chat_message.h
 * @brief Chat message domain model.
 */

#include "../common/types.h"
#include <cstdint>
#include <array>

namespace loralink::chat {

struct ChatMessage {
    uint32_t          id{0};
    Timestamp         timestamp{0};
    DeviceId          source_id{0};
    MessageDirection  direction{MessageDirection::Inbound};
    char              text[kMaxChatText]{};
    int16_t           rssi{0};
    bool              read{false};
    bool              delivered{false};
};

} // namespace loralink::chat
