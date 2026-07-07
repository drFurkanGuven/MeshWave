/**
 * @file cardputer_app.cpp
 * @brief Cardputer: keyboard UI, status bar, settings (Phase 8).
 */
#include "cardputer_app.h"
#include "app_core.h"
#include "board.h"
#include "../../lib/chat/chat_service.h"
#include "../../lib/drivers/radio/radio_factory.h"
#include "../../lib/display/cardputer_ui.h"
#include "../../lib/keyboard/keyboard_task.h"
#include "../../lib/common/logger.h"
#include "../../lib/common/queues.h"
#include <M5Cardputer.h>
#include <M5Unified.h>
#include <Arduino.h>
#include <memory>

namespace loralink::app {

using loralink::Logger;
using loralink::kGatewayId;
using loralink::QueueHandle;
using loralink::display::CardputerUi;
using loralink::display::StatusBarData;
using loralink::keyboard::KeyboardTask;
using loralink::keyboard::KeyEvent;
using loralink::keyboard::KeyEventType;
using loralink::keyboard::kFnSettings;
using loralink::radio::RadioFactory;

static CardputerUi s_ui;
static KeyboardTask s_kb;
static QueueHandle<KeyEvent> s_key_queue{8};
static bool s_started{false};

static void updateStatusBar(StatusBarData& bar, uint32_t now_ms) {
    auto* tr = transport();
    auto* rd = radio();
    bar.lora_ok = tr ? tr->stats().link_up : false;
    bar.rssi = rd ? rd->readRSSI() : 0;
    bar.snr = 0;
    bar.battery_pct = static_cast<uint8_t>(M5.Power.getBatteryLevel());
    if (bar.battery_pct > 100) bar.battery_pct = 100;

    const uint32_t mins = (now_ms / 1000u) / 60u;
    const uint32_t secs = (now_ms / 1000u) % 60u;
    snprintf(bar.clock_str, sizeof(bar.clock_str), "%02u:%02u",
             static_cast<unsigned>(mins % 100u), static_cast<unsigned>(secs));

    const auto* chat_svc = chat();
    bar.unread = chat_svc ? chat_svc->conversation().unreadCount() : 0;
}

static void handleKeyEvent(const KeyEvent& ev) {
    auto* chat_svc = chat();
    if (!chat_svc) return;

    if (ev.type == KeyEventType::FnCombo && (ev.fn_mask & kFnSettings)) {
        s_ui.showSettings(!s_ui.settingsVisible());
        return;
    }

    if (s_ui.settingsVisible()) return;

    if (ev.type == KeyEventType::Enter) {
        const char* text = s_kb.buffer().c_str();
        if (text[0]) {
            chat_svc->sendText(text, kGatewayId);
            s_kb.clearBuffer();
        }
        return;
    }

    if (ev.type == KeyEventType::Tab) {
        s_kb.clearBuffer();
    }
}

bool startCardputer() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);

    auto radio = RadioFactory::create();
    if (!radio) return false;

    AppOptions opts{};
    opts.role = DeviceRole::Terminal;
    opts.peer_id = kGatewayId;
    opts.send_probes = false;

    s_started = start(std::move(radio), opts);
    if (!s_started) return false;

    s_kb.begin(&s_key_queue);
    s_kb.onFnCombo([](uint16_t mask) {
        if (mask & kFnSettings) {
            s_ui.showSettings(!s_ui.settingsVisible());
        }
    });

    if (!s_ui.begin()) return false;

    auto* chat_svc = chat();
    if (chat_svc) {
        s_ui.setConversation(&chat_svc->conversation());
        chat_svc->conversation().markAllRead();
    }
    s_ui.setConfig(&config());

    Logger::info("CP", "UI ready — Enter=send Tab=clear Fn+S=settings");
    return true;
}

void cardputerLoop() {
    if (!s_started) {
        vTaskDelay(pdMS_TO_TICKS(200));
        return;
    }

    s_kb.poll();

    KeyEvent ev{};
    while (s_key_queue.tryReceive(ev)) {
        handleKeyEvent(ev);
    }

    auto* chat_svc = chat();
    if (chat_svc) {
        s_ui.setConversation(&chat_svc->conversation());
        s_ui.setInputBuffer(s_kb.buffer().c_str());
    }
    s_ui.setConfig(&config());

    const uint32_t now = millis();
    StatusBarData bar{};
    updateStatusBar(bar, now);
    s_ui.setStatus(bar);
    s_ui.poll(now);

    if (Serial.available() && chat_svc && !s_ui.settingsVisible()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length()) {
            chat_svc->sendText(line.c_str(), kGatewayId);
        }
    }

    vTaskDelay(pdMS_TO_TICKS(20));
}

} // namespace loralink::app
