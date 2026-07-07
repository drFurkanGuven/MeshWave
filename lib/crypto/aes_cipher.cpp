#include "aes_cipher.h"
#include <cstring>

namespace loralink::crypto {

void AesCipher::setKey(const std::array<uint8_t, 16>& key) {
    m_key = key;
}

bool AesCipher::encrypt(const uint8_t* in, size_t len, uint8_t* out, size_t out_cap,
                        const uint8_t*) const {
    if (!m_enabled || !in || !out || len > out_cap) return false;
    std::memcpy(out, in, len); // Phase 6: mbedTLS AES-CTR
    return true;
}

bool AesCipher::decrypt(const uint8_t* in, size_t len, uint8_t* out, size_t out_cap,
                        const uint8_t*) const {
    return encrypt(in, len, out, out_cap, nullptr);
}

} // namespace loralink::crypto
