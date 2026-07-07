# LoRaLink — Phase 2: Directory Tree

**Version:** 0.1.0  
**Status:** Complete — awaiting approval for Phase 3  
**Confirmed hardware:**

| Device | Board | Radio |
|--------|-------|-------|
| Device A | M5Stack Cardputer (ESP32-S3) | EBYTE E220-400T22S 433 MHz (UART) |
| Device B | LilyGO T3 LoRa32 V1.6.1 (ESP32) | SX1278 433 MHz (SPI) |

**Confirmed policy:** Message storage enabled on **both** devices (1000-msg circular buffer).

---

## Full Tree

```
loralink/
├── platformio.ini                 # Phase 3 — dual env: cardputer, gateway
├── README.md
├── partitions.csv                 # Phase 3 — LittleFS for message store
│
├── docs/
│   ├── PHASE1_ARCHITECTURE.md
│   └── PHASE2_DIRECTORY.md        # this file
│
├── boards/
│   ├── cardputer/
│   │   ├── board.h                # Pin map, feature flags
│   │   └── README.md
│   └── t3_v161/
│       ├── board.h                # T3 V1.6.1 SX1278 pins
│       └── README.md
│
├── data/                          # Gateway web assets (LittleFS image)
│   ├── index.html
│   ├── style.css
│   ├── app.js
│   └── README.md
│
├── src/
│   ├── main_cardputer.cpp         # Cardputer entry point
│   ├── main_gateway.cpp           # T3 gateway entry point
│   └── app/
│       ├── cardputer_app.h
│       ├── cardputer_app.cpp
│       ├── gateway_app.h
│       ├── gateway_app.cpp
│       └── README.md
│
├── include/
│   └── loralink/
│       └── version.h
│
└── lib/
    ├── common/
    │   ├── logger.h / logger.cpp
    │   ├── types.h
    │   ├── queues.h
    │   ├── errors.h
    │   └── README.md
    │
    ├── config/
    │   ├── config_store.h / config_store.cpp
    │   ├── defaults.h
    │   └── README.md
    │
    ├── drivers/
    │   ├── radio/
    │   │   ├── i_radio_driver.h     # IRadioDriver interface
    │   │   ├── radio_config.h
    │   │   ├── radio_packet.h
    │   │   ├── radio_factory.h / radio_factory.cpp
    │   │   └── README.md
    │   ├── e220/
    │   │   ├── e220_driver.h / e220_driver.cpp
    │   │   ├── e220_config.h
    │   │   └── README.md
    │   └── sx1278/
    │       ├── sx1278_driver.h / sx1278_driver.cpp
    │       └── README.md
    │
    ├── protocol/
    │   ├── packet.h
    │   ├── packet_types.h
    │   ├── packet_codec.h / packet_codec.cpp
    │   ├── crc16.h / crc16.cpp
    │   ├── sequence_manager.h / sequence_manager.cpp
    │   └── README.md
    │
    ├── transport/
    │   ├── reliable_transport.h / reliable_transport.cpp
    │   ├── ack_manager.h / ack_manager.cpp
    │   ├── retry_policy.h
    │   ├── dedup_cache.h / dedup_cache.cpp
    │   └── README.md
    │
    ├── routing/
    │   ├── router.h / router.cpp
    │   ├── address_table.h
    │   └── README.md
    │
    ├── crypto/
    │   ├── aes_cipher.h / aes_cipher.cpp
    │   ├── key_store.h / key_store.cpp
    │   ├── pairing_manager.h / pairing_manager.cpp
    │   └── README.md
    │
    ├── fragmentation/
    │   ├── fragmenter.h / fragmenter.cpp
    │   ├── reassembler.h / reassembler.cpp
    │   └── README.md
    │
    ├── chat/
    │   ├── chat_service.h / chat_service.cpp
    │   ├── chat_message.h
    │   ├── conversation_model.h / conversation_model.cpp
    │   └── README.md
    │
    ├── storage/
    │   ├── message_store.h / message_store.cpp
    │   ├── circular_index.h / circular_index.cpp
    │   └── README.md
    │
    ├── display/
    │   ├── display_renderer.h / display_renderer.cpp
    │   ├── cardputer_ui.h / cardputer_ui.cpp
    │   ├── gateway_oled.h / gateway_oled.cpp
    │   ├── theme.h
    │   └── README.md
    │
    ├── keyboard/
    │   ├── keyboard_task.h / keyboard_task.cpp
    │   ├── input_buffer.h
    │   └── README.md
    │
    ├── wifi/
    │   ├── wifi_manager.h / wifi_manager.cpp
    │   ├── ap_config.h
    │   └── README.md
    │
    ├── webserver/
    │   ├── static_server.h / static_server.cpp
    │   ├── api_handlers.h / api_handlers.cpp
    │   └── README.md
    │
    └── websocket/
        ├── ws_server.h / ws_server.cpp
        ├── ws_protocol.h
        └── README.md
```

---

## Module Dependency Graph

```
app (cardputer | gateway)
  ├── chat
  │     ├── storage          ← both devices
  │     ├── display / keyboard / wifi / webserver / websocket (per role)
  │     └── transport
  │           ├── protocol
  │           │     └── common
  │           ├── routing
  │           ├── crypto
  │           ├── fragmentation
  │           └── drivers/radio
  │                 ├── e220      (cardputer only)
  │                 └── sx1278    (gateway only)
  └── config
```

---

## Build Targets (Phase 3 preview)

| Environment | `src_filter` | Driver | App |
|-------------|--------------|--------|-----|
| `cardputer` | `main_cardputer.cpp`, `cardputer_app.*` | `e220` | Terminal UI |
| `gateway` | `main_gateway.cpp`, `gateway_app.*` | `sx1278` | WiFi + Web |

Shared libs: `protocol`, `transport`, `chat`, `storage`, `crypto`, `common`, `config`.

---

## Legacy Prototype

The following pre-LoRaLink files remain for reference and will be removed after Phase 7:

- `lilygo/` — simple T3 prototype
- `cardputer/` — simple Cardputer prototype
- `shared/` — superseded by `lib/protocol` + `lib/config`

---

## Phase 3 — Done

See [PHASE3_PLATFORMIO.md](PHASE3_PLATFORMIO.md).

---

*Phase 2 complete. Phase 3 complete. Awaiting Phase 4: Interfaces.*
