#pragma once

/**
 * @file conversation_model.h
 * @brief In-memory chat history model.
 */

#include "chat_message.h"
#include <cstddef>
#include <array>

namespace loralink::chat {

/**
 * @class ConversationModel
 * @brief Ring buffer of recent messages for UI display.
 */
class ConversationModel {
public:
    static constexpr size_t kDisplayCapacity = 50;

    void append(const ChatMessage& msg);
    size_t count() const { return m_count; }
    const ChatMessage& at(size_t index) const;
    size_t unreadCount() const;
    void markAllRead();

private:
    std::array<ChatMessage, kDisplayCapacity> m_messages{};
    size_t m_head{0};
    size_t m_count{0};
};

} // namespace loralink::chat
