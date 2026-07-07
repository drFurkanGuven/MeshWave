#include "pairing_manager.h"
#include <cstring>

namespace loralink::crypto {

PairingManager::PairingManager(KeyStore& store) : m_store(store) {}

bool PairingManager::pair(const char* code6) {
    if (!code6 || std::strlen(code6) != 6) return false;
    std::array<uint8_t, 16> key{};
    for (int i = 0; i < 6; i++) key[i] = static_cast<uint8_t>(code6[i]);
    return m_store.save(key);
}

bool PairingManager::unpair() { m_store.clear(); m_paired = false; return true; }
bool PairingManager::isPaired() const { return m_paired || m_store.hasKey(); }

} // namespace loralink::crypto
