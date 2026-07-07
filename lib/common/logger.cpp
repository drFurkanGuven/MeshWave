/**
 * @file logger.cpp
 * @brief Logger implementation (Phase 4 stub).
 */
#include "logger.h"
#include <Arduino.h>
#include <cstdio>

namespace loralink {

LogLevel Logger::s_level = LogLevel::Info;

void Logger::setLevel(LogLevel level) { s_level = level; }
LogLevel Logger::level() { return s_level; }

void Logger::logVa(LogLevel lvl, const char* tag, const char* fmt, va_list args) {
    if (static_cast<uint8_t>(lvl) < static_cast<uint8_t>(s_level)) return;

    static const char* kPrefix[] = {"I", "D", "W", "E"};
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    Serial.printf("[%s][%s] %s\n", kPrefix[static_cast<uint8_t>(lvl)], tag, buf);
}

void Logger::log(LogLevel lvl, const char* tag, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logVa(lvl, tag, fmt, args);
    va_end(args);
}

void Logger::info(const char* tag, const char* fmt, ...) {
    va_list args; va_start(args, fmt); logVa(LogLevel::Info, tag, fmt, args); va_end(args);
}
void Logger::debug(const char* tag, const char* fmt, ...) {
    va_list args; va_start(args, fmt); logVa(LogLevel::Debug, tag, fmt, args); va_end(args);
}
void Logger::warning(const char* tag, const char* fmt, ...) {
    va_list args; va_start(args, fmt); logVa(LogLevel::Warning, tag, fmt, args); va_end(args);
}
void Logger::error(const char* tag, const char* fmt, ...) {
    va_list args; va_start(args, fmt); logVa(LogLevel::Error, tag, fmt, args); va_end(args);
}

void Logger::dumpHex(LogLevel lvl, const char* tag, const uint8_t* data, size_t len) {
    if (!data || len == 0) return;
    size_t n = len > 32 ? 32 : len;
    char hex[97];
    size_t p = 0;
    for (size_t i = 0; i < n && p + 3 < sizeof(hex); i++) {
        p += snprintf(hex + p, sizeof(hex) - p, "%02X ", data[i]);
    }
    log(lvl, tag, "HEX[%u]: %s%s", static_cast<unsigned>(len), hex, len > 32 ? "..." : "");
}

} // namespace loralink
