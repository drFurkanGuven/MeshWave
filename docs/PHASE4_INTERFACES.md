# LoRaLink — Phase 4: Interfaces

**Status:** Complete — awaiting approval for Phase 5  
**Scope:** All layer headers + minimal stub `.cpp` for link-time symbols.

---

## Interface Map

| Layer | Key Types | Header |
|-------|-----------|--------|
| Common | `Result<T>`, `Logger`, `QueueHandle<T>` | `lib/common/` |
| Config | `AppConfig`, `ConfigStore` | `lib/config/` |
| Radio | `IRadioDriver`, `RadioFactory` | `lib/drivers/radio/` |
| E220 | `E220Driver` | `lib/drivers/e220/` |
| SX1278 | `Sx1278Driver` | `lib/drivers/sx1278/` |
| Protocol | `Packet`, `PacketCodec`, `crc16` | `lib/protocol/` |
| Transport | `ReliableTransport`, `AckManager` | `lib/transport/` |
| Routing | `Router`, `AddressTable` | `lib/routing/` |
| Crypto | `AesCipher`, `PairingManager` | `lib/crypto/` |
| Fragmentation | `Fragmenter`, `Reassembler` | `lib/fragmentation/` |
| Chat | `ChatService`, `ConversationModel` | `lib/chat/` |
| Storage | `MessageStore` (both devices) | `lib/storage/` |
| Display | `CardputerUi`, `GatewayOled` | `lib/display/` |
| Keyboard | `KeyboardTask`, `InputBuffer` | `lib/keyboard/` |
| WiFi | `WifiManager`, `ApConfig` | `lib/wifi/` |
| Web | `StaticServer`, `ApiHandlers` | `lib/webserver/` |
| WebSocket | `WsServer`, `WsProtocol` | `lib/websocket/` |

## Design Rules Enforced

1. **IRadioDriver** — only hardware boundary
2. **No `delay()`** — interfaces use `uint32_t now_ms` / `vTaskDelay` in tasks
3. **Queues** — `ChatService`, `ReliableTransport` accept `QueueHandle<T>*`
4. **RAII** — `QueueHandle`, `std::unique_ptr<IRadioDriver>`
5. **C++17** — `std::optional` avoided for ESP compatibility; `Result<T>` used instead

## Implemented in Phase 4 (working)

- `crc16` — full CRC-16/CCITT-FALSE
- `SequenceManager`, `RetryPolicy`, `AckManager`
- `Fragmenter`, `AddressTable`, `Router`
- `ConversationModel`, `InputBuffer`
- `Logger` — serial output

## Stubs (Phase 5–6)

- `PacketCodec::encode/decode`
- `ReliableTransport::send/poll`
- `E220Driver`, `Sx1278Driver` hardware
- `MessageStore` LittleFS persistence
- `ConfigStore` NVS load/save

## Umbrella Include

```cpp
#include <loralink/loralink.h>
```

---

*End of Phase 4. Awaiting confirmation to proceed to Phase 5: Drivers.*
