#pragma once

#include <Arduino.h>
#include <cstring>

// Basit metin protokolü: @LMB:<kaynak>:<mesaj>\n
// kaynak: "CP" (Cardputer) veya "LG" (LilyGo)

#define PROTO_MAGIC     "@LMB:"
#define PROTO_MAGIC_LEN 5
#define PROTO_MAX_LEN   180

inline bool protocol_pack(char* out, size_t outSize, const char* source, const char* message) {
    if (!out || !source || !message || outSize < 16) return false;
    int n = snprintf(out, outSize, "%s%s:%s\n", PROTO_MAGIC, source, message);
    return n > 0 && (size_t)n < outSize;
}

inline bool protocol_parse(const char* raw, size_t len, char* sourceOut, size_t srcSize,
                           char* msgOut, size_t msgSize) {
    if (!raw || len < PROTO_MAGIC_LEN + 4 || !sourceOut || !msgOut) return false;

    if (strncmp(raw, PROTO_MAGIC, PROTO_MAGIC_LEN) != 0) return false;

    const char* p = raw + PROTO_MAGIC_LEN;
    const char* colon = strchr(p, ':');
    if (!colon) return false;

    size_t srcLen = (size_t)(colon - p);
    if (srcLen == 0 || srcLen >= srcSize) return false;

    memcpy(sourceOut, p, srcLen);
    sourceOut[srcLen] = '\0';

    const char* msgStart = colon + 1;
    size_t msgLen = len - (size_t)(msgStart - raw);
    while (msgLen > 0 && (msgStart[msgLen - 1] == '\n' || msgStart[msgLen - 1] == '\r')) {
        msgLen--;
    }
    if (msgLen == 0 || msgLen >= msgSize) return false;

    memcpy(msgOut, msgStart, msgLen);
    msgOut[msgLen] = '\0';
    return true;
}
