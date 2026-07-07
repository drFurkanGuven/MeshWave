#pragma once

/**
 * @file message_record.h
 * @brief On-disk message record format for LittleFS store.
 */

#include "../common/types.h"
#include <cstdint>

namespace loralink::storage {

constexpr uint32_t kRecordMagic = 0x4D534752UL; // "MSGR"
constexpr size_t   kRecordSize  = 256;
constexpr const char* kDataFile = "/msgs.bin";

#pragma pack(push, 1)
struct StoredRecord {
    uint32_t magic{kRecordMagic};
    uint32_t id{0};
    uint32_t timestamp{0};
    uint16_t source_id{0};
    uint8_t  direction{0};
    uint8_t  text_len{0};
    char     text[240]{};
};
#pragma pack(pop)

static_assert(sizeof(StoredRecord) <= kRecordSize, "StoredRecord too large");

} // namespace loralink::storage
