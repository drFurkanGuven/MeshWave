#pragma once

/**
 * @file keyboard_task.h
 * @brief Cardputer keyboard polling task interface.
 */

#include "input_buffer.h"
#include "../common/queues.h"
#include "../chat/chat_message.h"
#include <functional>

namespace loralink::keyboard {

enum class KeyEventType : uint8_t {
    Char,
    Enter,
    Backspace,
    Tab,
    FnCombo,
};

constexpr uint16_t kFnSettings = 1 << 0;
constexpr uint16_t kFnWifi     = 1 << 1;

struct KeyEvent {
    KeyEventType type{KeyEventType::Char};
    char         ch{0};
    uint16_t     fn_mask{0};
};

/**
 * @class KeyboardTask
 * @brief Polls M5Cardputer keyboard, emits KeyEvents to queue.
 */
class KeyboardTask {
public:
    using FnHandler = std::function<void(uint16_t fn_mask)>;

    bool begin(QueueHandle<KeyEvent>* event_queue);
    void poll();
    InputBuffer& buffer() { return m_buffer; }
    const InputBuffer& buffer() const { return m_buffer; }
    void clearBuffer() { m_buffer.clear(); }

    void onFnCombo(FnHandler handler) { m_fn_handler = handler; }

private:
    QueueHandle<KeyEvent>* m_queue{nullptr};
    InputBuffer m_buffer;
    FnHandler m_fn_handler;
};

} // namespace loralink::keyboard
