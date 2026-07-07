#include "json_util.h"
#include <cstdio>
#include <cstring>

namespace loralink::webserver {

void appendJsonEscaped(String& out, const char* text) {
    if (!text) {
        out += "\"\"";
        return;
    }
    out += '"';
    for (const char* p = text; *p; ++p) {
        switch (*p) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<uint8_t>(*p) < 0x20) break;
                out += *p;
                break;
        }
    }
    out += '"';
}

bool extractJsonString(const char* json, const char* key, char* out, size_t out_len) {
    if (!json || !key || !out || out_len == 0) return false;

    char needle[32];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char* p = strstr(json, needle);
    if (!p) return false;

    p = strchr(p + strlen(needle), ':');
    if (!p) return false;
  p++;
    while (*p == ' ') p++;
    if (*p != '"') return false;
    p++;

    size_t i = 0;
    while (*p && *p != '"' && i + 1 < out_len) {
        if (*p == '\\' && p[1]) {
            p++;
            switch (*p) {
                case 'n': out[i++] = '\n'; break;
                case 'r': out[i++] = '\r'; break;
                case 't': out[i++] = '\t'; break;
                case '"': out[i++] = '"'; break;
                case '\\': out[i++] = '\\'; break;
                default: out[i++] = *p; break;
            }
        } else {
            out[i++] = *p;
        }
        p++;
    }
    out[i] = '\0';
    return i > 0;
}

} // namespace loralink::webserver
