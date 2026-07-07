#pragma once

/**
 * @file router.h
 * @brief Packet routing — point-to-point in v1, mesh-ready fields.
 */

#include "../protocol/packet.h"
#include "address_table.h"
#include "../common/types.h"

namespace loralink::routing {

/**
 * @class Router
 * @brief Decides if a packet is for us and sets outbound routing headers.
 */
class Router {
public:
    explicit Router(DeviceId local_id);

    bool isForUs(const protocol::Packet& pkt) const;
    void prepareOutbound(protocol::Packet& pkt, DeviceId dest) const;
    DeviceId localId() const { return m_local_id; }

    AddressTable& peers() { return m_table; }
    const AddressTable& peers() const { return m_table; }

private:
    DeviceId m_local_id;
    AddressTable m_table;
};

} // namespace loralink::routing
