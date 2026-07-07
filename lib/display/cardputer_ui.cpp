#include "cardputer_ui.h"
#include "theme.h"
#include <loralink/version.h>
#include <M5Cardputer.h>
#include <M5Unified.h>
#include <cstdio>
#include <cstring>

namespace loralink::display {

namespace {

constexpr int kWidth   = 240;
constexpr int kHeight  = 135;
constexpr int kStatusH = 16;
constexpr int kInputH  = 20;
constexpr int kChatY   = 17;
constexpr int kChatH   = kHeight - kStatusH - kInputH - 2;
constexpr int kInputY  = kHeight - kInputH;

uint32_t hashChat(const chat::ConversationModel* conv) {
    if (!conv) return 0;
    uint32_t h = static_cast<uint32_t>(conv->count());
    const size_t n = conv->count();
    const size_t start = n > 6 ? n - 6 : 0;
    for (size_t i = start; i < n; i++) {
        const auto& m = conv->at(i);
        h = h * 31u + m.id;
        h = h * 31u + static_cast<uint32_t>(m.timestamp);
    }
    return h;
}

const char* peerLabel(DeviceId id) {
    if (id == kCardputerId) return "CP";
    if (id == kGatewayId) return "GW";
    return "??";
}

} // namespace

bool CardputerUi::begin() {
    auto& d = M5Cardputer.Display;
    d.setRotation(1);
    d.fillScreen(Theme::bg);
    d.setTextSize(1);
    d.setTextWrap(false);
    m_last_render = 0;
    m_last_status_render = 0;
    m_last_chat_hash = 0;
    m_last_msg_count = 0;
    m_last_input[0] = '\0';
    return true;
}

void CardputerUi::setConversation(const chat::ConversationModel* model) {
    m_conversation = model;
}

void CardputerUi::setConfig(const config::AppConfig* cfg) {
    m_config = cfg;
}

void CardputerUi::setInputBuffer(const char* text) {
    if (!text) {
        m_input[0] = '\0';
        return;
    }
    snprintf(m_input, sizeof(m_input), "%s", text);
}

void CardputerUi::setStatus(const StatusBarData& status) {
    m_status = status;
}

void CardputerUi::showSettings(bool on) {
    if (m_settings != on) {
        m_settings = on;
        m_last_chat_hash = 0;
        m_last_msg_count = 0;
        M5Cardputer.Display.fillRect(0, kChatY, kWidth, kChatH + kInputH + 1, Theme::bg);
    }
}

void CardputerUi::poll(uint32_t now_ms) {
    auto& d = M5Cardputer.Display;

    if (now_ms - m_last_cursor_toggle >= 500) {
        m_cursor_on = !m_cursor_on;
        m_last_cursor_toggle = now_ms;
    }

    const bool status_due = (now_ms - m_last_status_render >= 1000);
    const uint32_t chat_hash = hashChat(m_conversation);
    const size_t msg_count = m_conversation ? m_conversation->count() : 0;
    const bool chat_changed = chat_hash != m_last_chat_hash || msg_count != m_last_msg_count;
    const bool input_changed = std::strcmp(m_input, m_last_input) != 0;
    const bool force = (now_ms - m_last_render) > 2000;

    if (status_due || force) {
        d.fillRect(0, 0, kWidth, kStatusH, Theme::surface);
        d.drawFastHLine(0, kStatusH, kWidth, Theme::muted);

        d.setTextColor(m_status.lora_ok ? Theme::success : Theme::warning);
        d.setCursor(2, 4);
        d.print(m_status.lora_ok ? "LoRa" : "----");

        d.setTextColor(Theme::text);
        d.setCursor(44, 4);
        if (m_status.rssi != 0) {
            d.printf("RSSI:%d", m_status.rssi);
        } else {
            d.print("RSSI:--");
        }

        d.setCursor(110, 4);
        d.printf("Bat:%u%%", m_status.battery_pct);

        d.setCursor(170, 4);
        d.print(m_status.clock_str);

        if (m_status.unread > 0) {
            d.setTextColor(Theme::accent);
            d.setCursor(220, 4);
            d.print("*");
        }

        m_last_status_render = now_ms;
    }

    if (m_settings) {
        if (chat_changed || force) {
            d.fillRect(0, kChatY, kWidth, kHeight - kChatY, Theme::bg);
            d.setTextColor(Theme::accent);
            d.setCursor(4, kChatY + 2);
            d.println("Settings (Fn+S close)");

            d.setTextColor(Theme::text);
            int y = kChatY + 16;
            auto line = [&](const char* label, const char* value) {
                d.setCursor(4, y);
                d.printf("%s %s", label, value);
                y += 12;
            };

            char buf[48];
            if (m_config) {
                line("Name:", m_config->device_name);
                snprintf(buf, sizeof(buf), "%04X", m_config->device_id);
                line("ID:", buf);
                snprintf(buf, sizeof(buf), "%.3f MHz", m_config->frequency_hz / 1000000.0f);
                line("Freq:", buf);
                snprintf(buf, sizeof(buf), "SF%u BW%uk", m_config->spreading_factor,
                         m_config->bandwidth_hz / 1000);
                line("LoRa:", buf);
                line("AES:", m_config->aes_enabled ? "on" : "off");
                snprintf(buf, sizeof(buf), "%s", m_config->wifi_ssid);
                line("WiFi:", buf[0] ? buf : "(none)");
            }
            snprintf(buf, sizeof(buf), "v%s", LORALINK_VERSION_STRING);
            line("About:", buf);

            m_last_chat_hash = chat_hash;
            m_last_msg_count = msg_count;
        }
        m_last_render = now_ms;
        return;
    }

    if (chat_changed || force) {
        d.fillRect(0, kChatY, kWidth, kChatH, Theme::bg);
        d.setTextColor(Theme::text);

        if (m_conversation && m_conversation->count() > 0) {
            const size_t n = m_conversation->count();
            const size_t max_lines = kChatH / 12;
            const size_t start = n > max_lines ? n - max_lines : 0;
            int y = kChatY + 2;
            for (size_t i = start; i < n; i++) {
                const auto& msg = m_conversation->at(i);
                const bool outbound = msg.direction == loralink::MessageDirection::Outbound;
                d.setTextColor(outbound ? Theme::accent : Theme::text);
                d.setCursor(2, y);

                char line[64];
                const char* who = outbound ? "You" : peerLabel(msg.source_id);
                snprintf(line, sizeof(line), "[%s] ", who);
                size_t prefix = std::strlen(line);
                size_t tlen = std::strlen(msg.text);
                const size_t room = sizeof(line) - prefix - 4;
                if (tlen > room) tlen = room;
                std::memcpy(line + prefix, msg.text, tlen);
                line[prefix + tlen] = '\0';
                if (tlen < std::strlen(msg.text)) {
                    std::strcat(line, "...");
                }
                d.print(line);
                y += 12;
            }
        } else {
            d.setTextColor(Theme::muted);
            d.setCursor(4, kChatY + 20);
            d.println("No messages yet");
            d.setCursor(4, kChatY + 34);
            d.println("Enter = send");
        }

        m_last_chat_hash = chat_hash;
        m_last_msg_count = msg_count;
    }

    if (input_changed || m_cursor_on || force) {
        d.fillRect(0, kInputY, kWidth, kInputH, Theme::surface);
        d.drawFastHLine(0, kInputY, kWidth, Theme::muted);
        d.setTextColor(Theme::text);
        d.setCursor(4, kInputY + 6);
        d.print("> ");
        d.print(m_input);
        if (m_cursor_on) {
            d.print("_");
        }
        std::strncpy(m_last_input, m_input, sizeof(m_last_input) - 1);
        m_last_input[sizeof(m_last_input) - 1] = '\0';
    }

    m_last_render = now_ms;
}

} // namespace loralink::display
