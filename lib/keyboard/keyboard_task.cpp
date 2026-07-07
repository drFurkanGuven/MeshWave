#include "keyboard_task.h"
#include <M5Cardputer.h>

namespace loralink::keyboard {

namespace {

void pushEvent(QueueHandle<KeyEvent>* q, KeyEventType type, char ch = 0, uint16_t fn = 0) {
    if (!q) return;
    KeyEvent ev{};
    ev.type = type;
    ev.ch = ch;
    ev.fn_mask = fn;
    q->trySend(ev);
}

uint16_t detectFnMask(const Keyboard_Class::KeysState& st) {
    uint16_t mask = 0;
    if (!st.fn) return 0;
    for (char c : st.word) {
        if (c == 's' || c == 'S') mask |= kFnSettings;
        if (c == 'w' || c == 'W') mask |= kFnWifi;
    }
    return mask;
}

} // namespace

bool KeyboardTask::begin(QueueHandle<KeyEvent>* event_queue) {
    m_queue = event_queue;
    m_buffer.clear();
    return m_queue != nullptr;
}

void KeyboardTask::poll() {
    M5Cardputer.update();
    if (!M5Cardputer.Keyboard.isChange()) return;

    auto st = M5Cardputer.Keyboard.keysState();
    const uint16_t fn_mask = detectFnMask(st);
    if (fn_mask) {
        pushEvent(m_queue, KeyEventType::FnCombo, 0, fn_mask);
        if (m_fn_handler) m_fn_handler(fn_mask);
        return;
    }

    if (st.tab) {
        m_buffer.clear();
        pushEvent(m_queue, KeyEventType::Tab);
        return;
    }

    if (st.enter) {
        pushEvent(m_queue, KeyEventType::Enter);
        return;
    }

    if (st.del) {
        m_buffer.backspace();
        pushEvent(m_queue, KeyEventType::Backspace);
        return;
    }

    if (st.space) {
        m_buffer.append(' ');
        pushEvent(m_queue, KeyEventType::Char, ' ');
    }

    for (char c : st.word) {
        if (c == '\b' || c == 127) {
            m_buffer.backspace();
            pushEvent(m_queue, KeyEventType::Backspace);
        } else if (c >= 32 && c < 127) {
            m_buffer.append(c);
            pushEvent(m_queue, KeyEventType::Char, c);
        }
    }
}

} // namespace loralink::keyboard
