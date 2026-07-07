#include "key_store.h"

namespace loralink::crypto {

bool KeyStore::load(std::array<uint8_t, 16>&) const { return false; }
bool KeyStore::save(const std::array<uint8_t, 16>&) { return false; }
bool KeyStore::hasKey() const { return false; }
void KeyStore::clear() {}

} // namespace loralink::crypto
