#pragma once

/**
 * @file cardputer_ui.h
 * @brief Full chat UI for Cardputer ST7789V2.
 */

#include "../config/config_store.h"
#include "display_renderer.h"
#include "../chat/conversation_model.h"
#include "../chat/chat_message.h"

namespace loralink::display {

/**
 * @class CardputerUi
 * @brief Cardputer-specific display task controller.
 */
class CardputerUi {
public:
    bool begin();
    void poll(uint32_t now_ms);

    void setConversation(const chat::ConversationModel* model);
    void setConfig(const config::AppConfig* cfg);
    void setInputBuffer(const char* text);
    void setStatus(const StatusBarData& status);
    void showSettings(bool on);
    bool settingsVisible() const { return m_settings; }

private:
    const chat::ConversationModel* m_conversation{nullptr};
    const config::AppConfig* m_config{nullptr};
    char m_input[kMaxChatText]{};
    StatusBarData m_status{};
    bool m_settings{false};
    uint32_t m_last_render{0};
    uint32_t m_last_status_render{0};
    uint32_t m_last_chat_hash{0};
    size_t m_last_msg_count{0};
    char m_last_input[kMaxChatText]{};
    bool m_cursor_on{true};
    uint32_t m_last_cursor_toggle{0};
};

} // namespace loralink::display
