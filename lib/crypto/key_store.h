#pragma once

/**
 * @file key_store.h
 * @brief Secure storage of network encryption key.
 */

#include <array>
#include <cstdint>

namespace loralink::crypto {

class KeyStore {
public:
    bool load(std::array<uint8_t, 16>& key) const;
    bool save(const std::array<uint8_t, 16>& key);
    bool hasKey() const;
    void clear();
};

} // namespace loralink::crypto
