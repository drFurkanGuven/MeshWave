#pragma once

/**
 * @file logger.h
 * @brief Leveled serial logging for LoRaLink.
 */

#include <cstdarg>
#include <cstdint>
#include <cstddef>

namespace loralink {

enum class LogLevel : uint8_t {
    Info    = 0,
    Debug   = 1,
    Warning = 2,
    Error   = 3,
};

/**
 * @class Logger
 * @brief Thread-safe leveled logger writing to Serial.
 */
class Logger {
public:
    static void setLevel(LogLevel level);
    static LogLevel level();

    static void info(const char* tag, const char* fmt, ...);
    static void debug(const char* tag, const char* fmt, ...);
    static void warning(const char* tag, const char* fmt, ...);
    static void error(const char* tag, const char* fmt, ...);

    static void log(LogLevel lvl, const char* tag, const char* fmt, ...);
    static void dumpHex(LogLevel lvl, const char* tag, const uint8_t* data, size_t len);

private:
    static void logVa(LogLevel lvl, const char* tag, const char* fmt, va_list args);
    static LogLevel s_level;
};

} // namespace loralink
