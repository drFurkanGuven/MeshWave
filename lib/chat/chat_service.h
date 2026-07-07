#pragma once

/**
 * @file chat_service.h
 * @brief Chat orchestration — bridge between UI/WS and transport.
 */

#include "chat_message.h"
#include "conversation_model.h"
#include "../transport/reliable_transport.h"
#include "../storage/message_store.h"
#include "../protocol/packet.h"
#include "../common/queues.h"
#include <functional>

namespace loralink::chat {

using MessageListener = std::function<void(const ChatMessage&)>;

/**
 * @class ChatService
 * @brief Sends and receives chat messages via ReliableTransport.
 */
class ChatService {
public:
    ChatService(transport::ReliableTransport& transport,
                storage::MessageStore& store,
                DeviceId local_id);

    bool begin();
    void poll();

    bool sendText(const char* text, DeviceId dest);
    void processPacket(const protocol::Packet& pkt);
    void onInbound(const ChatMessage& msg);

    ConversationModel& conversation() { return m_model; }
    const ConversationModel& conversation() const { return m_model; }

    void setInboundQueue(QueueHandle<protocol::Packet>* q);
    void setMessageListener(MessageListener listener);
    uint32_t nextMessageId();

private:
    transport::ReliableTransport& m_transport;
    storage::MessageStore& m_store;
    DeviceId m_local_id;
    ConversationModel m_model;
    QueueHandle<protocol::Packet>* m_inbound{nullptr};
    MessageListener m_listener;
    uint32_t m_next_id{1};
};

} // namespace loralink::chat
