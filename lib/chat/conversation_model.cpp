#include "conversation_model.h"

namespace loralink::chat {

void ConversationModel::append(const ChatMessage& msg) {
  size_t idx = (m_head + m_count) % kDisplayCapacity;
    if (m_count < kDisplayCapacity) {
        m_messages[idx] = msg;
        m_count++;
    } else {
        m_messages[m_head] = msg;
        m_head = (m_head + 1) % kDisplayCapacity;
    }
}

const ChatMessage& ConversationModel::at(size_t index) const {
    size_t i = (m_head + index) % kDisplayCapacity;
    return m_messages[i];
}

size_t ConversationModel::unreadCount() const {
    size_t n = 0;
    for (size_t i = 0; i < m_count; i++) {
        if (!at(i).read) n++;
    }
    return n;
}

void ConversationModel::markAllRead() {
    for (size_t i = 0; i < m_count; i++) {
        size_t idx = (m_head + i) % kDisplayCapacity;
        m_messages[idx].read = true;
    }
}

} // namespace loralink::chat
