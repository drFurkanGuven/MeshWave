#pragma once

/**
 * @file address_table.h
 * @brief Known device address registry (point-to-point v1).
 */

#include "../common/types.h"
#include <cstdint>
#include <array>

namespace loralink::routing {

struct DeviceEntry {
    DeviceId id{0};
    char     name[kMaxDeviceName]{};
    bool     online{false};
    int16_t  last_rssi{0};
};

/**
 * @class AddressTable
 * @brief Static table of known peers (max 8 devices in v1).
 */
class AddressTable {
public:
    static constexpr size_t kMaxDevices = 8;

    bool add(DeviceId id, const char* name);
    bool remove(DeviceId id);
    const DeviceEntry* find(DeviceId id) const;
    void setOnline(DeviceId id, bool online, int16_t rssi = 0);
    size_t count() const { return m_count; }

private:
    std::array<DeviceEntry, kMaxDevices> m_entries{};
    size_t m_count{0};
};

} // namespace loralink::routing
