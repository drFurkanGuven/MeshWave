#pragma once

/**
 * @file pairing_manager.h
 * @brief Simple pairing code → shared AES key derivation.
 */

#include "key_store.h"
#include <cstdint>

namespace loralink::crypto {

/**
 * @class PairingManager
 * @brief Derives AES-128 key from 6-digit pairing code.
 */
class PairingManager {
public:
  explicit PairingManager(KeyStore& store);

    bool pair(const char* code6);
    bool unpair();
    bool isPaired() const;

private:
    KeyStore& m_store;
    bool m_paired{false};
};

} // namespace loralink::crypto
