#pragma once

#include <WString.h>

namespace loralink::webserver {

void appendJsonEscaped(String& out, const char* text);
bool extractJsonString(const char* json, const char* key, char* out, size_t out_len);

} // namespace loralink::webserver
