/**
 * @file chat_service.cpp
 */
#include "chat_service.h"
#include "../common/logger.h"
#include <Arduino.h>
#include <cstring>

namespace loralink::chat {

using loralink::Logger;

ChatService::ChatService(transport::ReliableTransport& transport,
                         storage::MessageStore& store,
                         DeviceId local_id)
    : m_transport(transport), m_store(store), m_local_id(local_id) {}

bool ChatService::begin() {
    if (!m_store.begin()) return false;
    m_store.loadRecent(20, [this](const ChatMessage& msg) {
        m_model.append(msg);
        if (msg.id >= m_next_id) m_next_id = msg.id + 1;
    });
    Logger::info("CHAT", "loaded %u messages from store", m_model.count());
    return true;
}

void ChatService::setInboundQueue(QueueHandle<protocol::Packet>* q) {
    m_inbound = q;
}

void ChatService::setMessageListener(MessageListener listener) {
    m_listener = std::move(listener);
}

uint32_t ChatService::nextMessageId() {
    return m_next_id++;
}

void ChatService::onInbound(const ChatMessage& msg) {
    ChatMessage copy = msg;
    if (copy.id == 0) copy.id = nextMessageId();
    m_model.append(copy);
    m_store.append(copy);
    Logger::info("CHAT", "[%s] %04X: %s",
                 copy.direction == MessageDirection::Outbound ? "OUT" : "IN ",
                 copy.source_id, copy.text);
    if (m_listener) m_listener(copy);
}

void ChatService::processPacket(const protocol::Packet& pkt) {
    if (pkt.type != protocol::PacketType::Message) return;

    ChatMessage msg{};
    msg.id = nextMessageId();
    msg.timestamp = pkt.timestamp ? pkt.timestamp : static_cast<Timestamp>(millis() / 1000UL);
    msg.source_id = pkt.source_id;
    msg.direction = MessageDirection::Inbound;
    msg.read = false;

    size_t len = pkt.payload_len;
    if (len >= kMaxChatText) len = kMaxChatText - 1;
    std::memcpy(msg.text, pkt.payload.data(), len);
    msg.text[len] = '\0';

    onInbound(msg);
}

void ChatService::poll() {
    if (!m_inbound) return;
    protocol::Packet pkt;
    while (m_inbound->receive(pkt, 0)) {
        processPacket(pkt);
    }
}

bool ChatService::sendText(const char* text, DeviceId dest) {
    if (!text || text[0] == '\0') return false;

    const size_t len = std::strlen(text);
    if (!m_transport.sendMessage(dest, text, len)) {
        Logger::error("CHAT", "send failed");
        return false;
    }

    ChatMessage msg{};
    msg.id = nextMessageId();
    msg.timestamp = static_cast<Timestamp>(millis() / 1000UL);
    msg.source_id = m_local_id;
    msg.direction = MessageDirection::Outbound;
    msg.delivered = true;
  std::strncpy(msg.text, text, sizeof(msg.text) - 1);
    onInbound(msg);
    return true;
}

} // namespace loralink::chat
