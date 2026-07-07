#pragma once

/**
 * @file input_buffer.h
 * @brief Line editor buffer for keyboard input.
 */

#include "../common/types.h"
#include <cstddef>
#include <cstring>

namespace loralink::keyboard {

class InputBuffer {
public:
    void clear() { m_len = 0; m_buf[0] = '\0'; }
    size_t length() const { return m_len; }
    const char* c_str() const { return m_buf; }

    bool append(char c);
    bool backspace();
    bool set(const char* text);

private:
    char m_buf[kMaxChatText]{};
    size_t m_len{0};
};

} // namespace loralink::keyboard
