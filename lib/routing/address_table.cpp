#include "address_table.h"
#include <cstring>

namespace loralink::routing {

bool AddressTable::add(DeviceId id, const char* name) {
    if (m_count >= kMaxDevices) return false;
    m_entries[m_count] = {id, {}, false, 0};
    if (name) std::strncpy(m_entries[m_count].name, name, kMaxDeviceName - 1);
    m_count++;
    return true;
}

bool AddressTable::remove(DeviceId id) {
    for (size_t i = 0; i < m_count; i++) {
        if (m_entries[i].id == id) {
            for (size_t j = i + 1; j < m_count; j++) m_entries[j - 1] = m_entries[j];
            m_count--;
            return true;
        }
    }
    return false;
}

const DeviceEntry* AddressTable::find(DeviceId id) const {
    for (size_t i = 0; i < m_count; i++) {
        if (m_entries[i].id == id) return &m_entries[i];
    }
    return nullptr;
}

void AddressTable::setOnline(DeviceId id, bool online, int16_t rssi) {
    for (auto& e : m_entries) {
        if (e.id == id) { e.online = online; e.last_rssi = rssi; return; }
    }
}

} // namespace loralink::routing
