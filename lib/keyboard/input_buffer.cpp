#include "input_buffer.h"

namespace loralink::keyboard {

bool InputBuffer::append(char c) {
    if (m_len + 1 >= kMaxChatText) return false;
    m_buf[m_len++] = c;
    m_buf[m_len] = '\0';
    return true;
}

bool InputBuffer::backspace() {
    if (m_len == 0) return false;
    m_buf[--m_len] = '\0';
    return true;
}

bool InputBuffer::set(const char* text) {
    if (!text) return false;
    size_t n = std::strlen(text);
    if (n >= kMaxChatText) return false;
    std::memcpy(m_buf, text, n + 1);
    m_len = n;
    return true;
}

} // namespace loralink::keyboard
