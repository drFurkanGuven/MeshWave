#pragma once

/**
 * @file message_store.h
 * @brief Persistent circular message storage (both devices).
 */

#include "circular_index.h"
#include "../chat/chat_message.h"
#include <functional>

namespace loralink::storage {

/**
 * @class MessageStore
 * @brief LittleFS-backed 1000-message circular buffer.
 */
class MessageStore {
public:
    using LoadCallback = std::function<void(const chat::ChatMessage&)>;

    bool begin();
    bool append(const chat::ChatMessage& msg);
    bool loadAll(const LoadCallback& cb) const;
    bool loadRecent(size_t n, const LoadCallback& cb) const;
    size_t count() const { return m_index.count; }
    bool clear();

private:
    CircularIndex m_index{};
    bool m_ready{false};
};

} // namespace loralink::storage
