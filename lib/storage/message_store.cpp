/**
 * @file message_store.cpp
 * @brief LittleFS circular message store (1000 messages, both devices).
 */
#include "message_store.h"
#include "message_record.h"
#include "../common/logger.h"
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <cstring>

namespace loralink::storage {

using loralink::Logger;

namespace {

constexpr const char* kIdxNs = "loralink_msg";
constexpr const char* kKeyHead = "head";
constexpr const char* kKeyCount = "count";
constexpr const char* kKeyNextId = "next_id";

bool loadIndex(CircularIndex& idx) {
    Preferences prefs;
    if (!prefs.begin(kIdxNs, true)) return false;
    idx.head = prefs.getUInt(kKeyHead, 0);
    idx.count = prefs.getUInt(kKeyCount, 0);
    idx.next_id = prefs.getUInt(kKeyNextId, 1);
    if (idx.count > CircularIndex::kCapacity) idx.count = CircularIndex::kCapacity;
    if (idx.head >= CircularIndex::kCapacity) idx.head = 0;
    prefs.end();
    return true;
}

bool saveIndex(const CircularIndex& idx) {
    Preferences prefs;
    if (!prefs.begin(kIdxNs, false)) return false;
    prefs.putUInt(kKeyHead, idx.head);
    prefs.putUInt(kKeyCount, idx.count);
    prefs.putUInt(kKeyNextId, idx.next_id);
    prefs.end();
    return true;
}

StoredRecord toRecord(const chat::ChatMessage& msg) {
    StoredRecord rec{};
    rec.magic = kRecordMagic;
    rec.id = msg.id;
    rec.timestamp = msg.timestamp;
    rec.source_id = msg.source_id;
    rec.direction = static_cast<uint8_t>(msg.direction);
    size_t len = std::strlen(msg.text);
    if (len > sizeof(rec.text)) len = sizeof(rec.text);
    rec.text_len = static_cast<uint8_t>(len);
    std::memcpy(rec.text, msg.text, len);
    return rec;
}

chat::ChatMessage fromRecord(const StoredRecord& rec) {
    chat::ChatMessage msg{};
    msg.id = rec.id;
    msg.timestamp = rec.timestamp;
    msg.source_id = rec.source_id;
    msg.direction = static_cast<loralink::MessageDirection>(rec.direction);
    size_t len = rec.text_len;
    if (len >= sizeof(msg.text)) len = sizeof(msg.text) - 1;
    std::memcpy(msg.text, rec.text, len);
    msg.text[len] = '\0';
    msg.read = true;
    return msg;
}

} // namespace

bool MessageStore::begin() {
    if (!LittleFS.begin(true)) {
        Logger::error("STORE", "LittleFS mount failed");
        return false;
    }
    if (!LittleFS.exists(kDataFile)) {
        File f = LittleFS.open(kDataFile, FILE_WRITE);
        if (!f) {
            Logger::error("STORE", "create %s failed", kDataFile);
            return false;
        }
        StoredRecord empty{};
        for (size_t i = 0; i < CircularIndex::kCapacity; i++) {
            f.write(reinterpret_cast<uint8_t*>(&empty), kRecordSize);
        }
        f.close();
    }
    loadIndex(m_index);
    m_ready = true;
    Logger::info("STORE", "ready count=%u next_id=%u", m_index.count, m_index.next_id);
    return true;
}

bool MessageStore::append(const chat::ChatMessage& msg) {
    if (!m_ready) return false;

    chat::ChatMessage stored = msg;
    if (stored.id == 0) stored.id = m_index.next_id;

    const uint32_t slot = m_index.pushIndex();
    File f = LittleFS.open(kDataFile, "r+");
    if (!f) return false;

    const StoredRecord rec = toRecord(stored);
    f.seek(slot * kRecordSize);
    uint8_t buf[kRecordSize]{};
    std::memcpy(buf, &rec, sizeof(rec));
    f.write(buf, kRecordSize);
    f.close();

    m_index.advance();
    saveIndex(m_index);
    return true;
}

bool MessageStore::loadAll(const LoadCallback& cb) const {
    if (!m_ready || !cb) return false;
    return loadRecent(m_index.count, cb);
}

bool MessageStore::loadRecent(size_t n, const LoadCallback& cb) const {
    if (!m_ready || !cb || n == 0) return false;
    if (n > m_index.count) n = m_index.count;

    File f = LittleFS.open(kDataFile, FILE_READ);
    if (!f) return false;

    for (size_t i = 0; i < n; i++) {
        const size_t logical = m_index.count - n + i;
        uint32_t slot = (m_index.head + logical) % CircularIndex::kCapacity;
        f.seek(slot * kRecordSize);
        StoredRecord rec{};
        if (f.read(reinterpret_cast<uint8_t*>(&rec), kRecordSize) != static_cast<int>(kRecordSize)) {
            continue;
        }
        if (rec.magic != kRecordMagic) continue;
        cb(fromRecord(rec));
    }
    f.close();
    return true;
}

bool MessageStore::clear() {
    m_index = {};
    saveIndex(m_index);
    return begin();
}

} // namespace loralink::storage
