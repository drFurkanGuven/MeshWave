#pragma once

/**
 * @file aes_cipher.h
 * @brief Optional AES-128-CTR payload encryption.
 */

#include <cstddef>
#include <cstdint>
#include <array>

namespace loralink::crypto {

/**
 * @class AesCipher
 * @brief AES-128-CTR encrypt/decrypt for packet payloads.
 */
class AesCipher {
public:
    void setKey(const std::array<uint8_t, 16>& key);
    bool enabled() const { return m_enabled; }
    void setEnabled(bool on) { m_enabled = on; }

    bool encrypt(const uint8_t* in, size_t len, uint8_t* out, size_t out_cap,
                 const uint8_t nonce[8]) const;
    bool decrypt(const uint8_t* in, size_t len, uint8_t* out, size_t out_cap,
                 const uint8_t nonce[8]) const;

private:
    std::array<uint8_t, 16> m_key{};
    bool m_enabled{false};
};

} // namespace loralink::crypto
