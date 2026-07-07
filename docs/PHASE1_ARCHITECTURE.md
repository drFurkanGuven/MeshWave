# LoRaLink — Phase 1: System Architecture

**Version:** 0.1.0-draft  
**Status:** Superseded by Phase 2 hardware confirmation — see §2  
**Author:** LoRaLink Core Team  
**Date:** 2026-07-07

---

## 1. Executive Summary

LoRaLink is a **local-only, bidirectional LoRa chat system** designed as a clean-room alternative to Meshtastic. Two heterogeneous devices communicate over a custom binary protocol with reliability, optional encryption, and rich UIs on both ends.

| Device | Role | MCU | Radio | Human Interface | Network Bridge |
|--------|------|-----|-------|-----------------|----------------|
| **Device A** | Field terminal | ESP32-S3 (Cardputer) | E220-400T22S (UART / LLCC68) | Keyboard + 240×135 display | Optional WiFi AP (remote keyboard) |
| **Device B** | Gateway / hub | ESP32 (T3 LoRa32 V1.6.1) | SX1278 (SPI) | OLED 128×64 (status) | WiFi AP + browser chat |

**Design principle:** Application logic never touches hardware. All radio access flows through `IRadioDriver`.

**Storage:** 1000-message circular buffer on **both** devices.

---

## 2. Hardware Reality & Architectural Constraints

### 2.1 Heterogeneous Radio Stack (Critical)

This is the single hardest constraint in the project.

| Property | E220-400T22S | SX1278 (T3 SPI) |
|----------|--------------|-----------------|
| Host interface | UART (transparent / config modes) | SPI + DIO0 IRQ |
| Chip inside | LLCC68 | SX1278 |
| LoRa PHY control | **Module firmware** (opaque) | **Host firmware** (full control) |
| Max payload (typical) | 200 bytes (sub-packet setting) | 255 bytes |
| Config mechanism | DIP switches M0/M1 + register commands | LoRa.h / register writes |

**Decision:** The `IRadioDriver` API operates on **opaque LoRa frames** at the transport boundary. Each driver is responsible for mapping `LoRaFrame` ↔ hardware.

**Interoperability requirement:** Both radios MUST be configured to identical LoRa PHY parameters:

| Parameter | Initial default | Notes |
|-----------|-----------------|-------|
| Frequency | 433.125 MHz | E220 channel 0x17 |
| Bandwidth | 125 kHz | |
| Spreading Factor | 12 | Matches E220 2.4 kbps air rate |
| Coding Rate | 4/5 | |
| Sync Word | 0x12 | SX1278 8-bit private network |
| Preamble | 8 symbols | |
| CRC | Enabled on SX1278; E220 handles internally | |
| TX Power | 14 dBm (T3), 22 dBm (E220, software limited) | |

**Risk:** LLCC68 ↔ SX1278 interoperability requires matched PHY; `CONFIG_SYNC` + `PING`/`PONG` validate link.

### 2.2 T3 LoRa32 V1.6.1 Specifics

- **MCU:** ESP32-PICO-D4, 4 MB flash
- **Radio:** SX1278 433 MHz via SPI
- **PMU:** None (USB or LiPo direct)
- **Display:** SSD1306 128×64 I2C (GPIO 21/22)
- **SPI pins:** CS=18, RST=23, DIO0=26, SCK=5, MISO=19, MOSI=27
- **PlatformIO board:** `ttgo-lora32-v21`

### 2.3 Cardputer Specifics

- **MCU:** ESP32-S3
- **Grove UART:** TX=GPIO1, RX=GPIO2 @ 9600/115200 (configured to match E220)
- **E220 module:** EBYTE E220-400T22S on M5 Grove unit — DIP switches M0/M1 (not GPIO)
- **Display:** ST7789V2 240×135
- **Keyboard:** 56-key matrix via M5Cardputer library (polled in dedicated task)

---

## 3. Layered Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     APPLICATION LAYER                            │
│  AppContext · Device personality (CardputerApp / GatewayApp)    │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                       CHAT LAYER                                 │
│  ChatService · ConversationModel · MessageComposer              │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                     PROTOCOL LAYER                               │
│  PacketCodec · PacketTypes · SequenceManager · CRC16             │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                     ROUTING LAYER                                │
│  Router · AddressTable · TTL/Hop policy (point-to-point v1)     │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                    ENCRYPTION LAYER                              │
│  Aes128Cipher · KeyStore · PairingManager (optional)            │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                  FRAGMENTATION LAYER                             │
│  Fragmenter · Reassembler · (payload > MTU)                     │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                    TRANSPORT LAYER                               │
│  ReliableTransport · AckManager · RetryPolicy · DedupCache      │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                  RADIO DRIVER LAYER                              │
│  IRadioDriver · E220Driver · SX1262Driver · RadioFactory        │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                    HARDWARE LAYER                                │
│  Board HAL · GPIO · UART · SPI · I2C · WiFi · Preferences       │
└─────────────────────────────────────────────────────────────────┘
```

### 3.1 Layer Independence Rules

1. **Downward dependencies only** — upper layers depend on interfaces, not implementations.
2. **No `delay()`** — all timing via `vTaskDelay`, timers, or event groups.
3. **No cross-layer globals** — communication via queues, event groups, and dependency injection.
4. **Board-specific code lives in `src/boards/`** — not scattered in protocol/chat code.

---

## 4. Packet Protocol Design

### 4.1 Wire Format (Fixed Header + Variable Payload)

All multi-byte integers: **little-endian**.

```
Offset  Size  Field
──────  ────  ─────────────────
0       2     magic          0x4C4B ("LK")
2       1     version        0x01
3       1     packet_type    see §4.2
4       1     flags          bitfield
5       2     sequence       uint16
7       2     source_id      uint16
9       2     dest_id        uint16
11      1     hop_count      uint8
12      1     ttl            uint8
13      4     timestamp      uint32 (Unix epoch, device-local OK for v1)
17      2     payload_len    uint16
19      N     payload        N = payload_len (max 180 bytes v1)
19+N    2     crc16          CRC-16/CCITT-FALSE over bytes [0..18+N]
```

**v1 MTU rationale:** E220 sub-packet 200 bytes − header 21 − CRC 2 − margin ≈ **180 bytes payload**. Fragmentation handles larger chat messages.

### 4.2 Packet Types

| Value | Name | Direction | Purpose |
|-------|------|-----------|---------|
| 0x01 | HELLO | Bidirectional | Capability exchange on boot |
| 0x02 | PING | Bidirectional | Link test |
| 0x03 | PONG | Bidirectional | Ping response |
| 0x10 | MESSAGE | Bidirectional | Chat payload (UTF-8) |
| 0x11 | ACK | Bidirectional | Reliable delivery confirm |
| 0x12 | NACK | Bidirectional | Negative acknowledge |
| 0x20 | DEVICE_INFO | Bidirectional | Name, firmware version, battery |
| 0x30 | KEEPALIVE | Bidirectional | Link maintenance |
| 0x40 | ERROR | Bidirectional | Error reporting |
| 0x50 | CONFIG_SYNC | Bidirectional | PHY parameter verification |

### 4.3 Flags Bitfield

| Bit | Name | Meaning |
|-----|------|---------|
| 0 | ACK_REQ | Sender wants ACK |
| 1 | FRAGMENT | Payload is a fragment |
| 2 | LAST_FRAG | Final fragment |
| 3 | ENCRYPTED | Payload is AES-128 encrypted |
| 4 | BROADCAST | dest_id = 0xFFFF |
| 5-7 | Reserved | Must be zero |

### 4.4 Device IDs (v1)

| ID | Device |
|----|--------|
| 0x0001 | Cardputer (default) |
| 0x0002 | T-Beam Gateway (default) |
| 0xFFFF | Broadcast |

Configurable via Preferences.

---

## 5. Reliability Model

### 5.1 ReliableTransport State Machine

```
         ┌──────────┐
         │  IDLE    │
         └────┬─────┘
              │ send(ACK_REQ)
              ▼
         ┌──────────┐     timeout      ┌──────────┐
         │ WAIT_ACK │ ───────────────► │ RETRY    │
         └────┬─────┘                  └────┬─────┘
              │ ACK received                │ retry < max
              ▼                             │
         ┌──────────┐ ◄─────────────────────┘
         │ DELIVERED│
         └──────────┘
```

**Parameters (configurable):**

| Parameter | Default |
|-----------|---------|
| ACK timeout | 3000 ms |
| Max retries | 3 |
| Retry backoff | exponential (1×, 2×, 4×) |
| Dedup window | 64 sequence numbers |
| Duplicate detection | Per (source_id, sequence) |

### 5.2 CRC

- Algorithm: **CRC-16/CCITT-FALSE** (poly 0x1021, init 0xFFFF)
- Reject packet on mismatch, increment `rx_crc_errors` counter

---

## 6. IRadioDriver Interface

```cpp
namespace loralink::radio {

struct RadioConfig {
    uint32_t frequency_hz;
    uint8_t  spreading_factor;
    uint32_t bandwidth_hz;
    uint8_t  coding_rate;      // 5 = 4/5
    uint16_t sync_word;        // chip-specific mapping in driver
    int8_t   tx_power_dbm;
    uint16_t preamble_length;
};

struct RxMetadata {
    int16_t rssi;
    int8_t  snr;
    uint32_t rx_timestamp_ms;
};

struct RadioPacket {
    std::array<uint8_t, 255> data;
    size_t length;
    RxMetadata meta;
};

class IRadioDriver {
public:
    virtual ~IRadioDriver() = default;

    virtual bool initialize() = 0;
    virtual bool setConfig(const RadioConfig& cfg) = 0;
    virtual bool getConfig(RadioConfig& cfg) const = 0;

    virtual bool sendPacket(const uint8_t* data, size_t len) = 0;
    virtual bool receivePacket(RadioPacket& out, uint32_t timeout_ms) = 0;

    virtual bool sleep() = 0;
    virtual bool wake() = 0;

    virtual int16_t readRSSI() = 0;
    virtual bool isChannelFree() = 0;   // LBT if supported

    virtual const char* driverName() const = 0;
};

} // namespace
```

**Factory:** `RadioFactory::create(BoardType)` returns `std::unique_ptr<IRadioDriver>`.

---

## 7. FreeRTOS Task Architecture

### 7.1 Cardputer Task Map

| Task | Priority | Stack | Core | Responsibility |
|------|----------|-------|------|----------------|
| `radio_task` | 5 | 8 KB | 1 | Poll E220 RX, push to `rx_queue` |
| `protocol_task` | 4 | 8 KB | 1 | Decode, ACK, retransmit |
| `chat_task` | 3 | 6 KB | 1 | Message model updates |
| `display_task` | 2 | 6 KB | 1 | Render UI at 10-15 FPS |
| `keyboard_task` | 3 | 4 KB | 0 | Poll keys → `input_queue` |
| `wifi_task` | 2 | 6 KB | 0 | Optional AP + remote keyboard |
| `storage_task` | 1 | 4 KB | 0 | Flash write (messages) |

### 7.2 T-Beam Gateway Task Map

| Task | Priority | Stack | Core | Responsibility |
|------|----------|-------|------|----------------|
| `radio_task` | 5 | 8 KB | 1 | SX1262 IRQ-driven RX |
| `protocol_task` | 4 | 8 KB | 1 | Same stack as Cardputer |
| `chat_task` | 3 | 6 KB | 1 | Bridge chat ↔ WebSocket |
| `wifi_task` | 3 | 6 KB | 0 | AP management |
| `webserver_task` | 2 | 8 KB | 0 | HTTP static assets |
| `websocket_task` | 4 | 6 KB | 0 | WS chat relay |
| `display_task` | 1 | 4 KB | 0 | OLED status bar |
| `storage_task` | 1 | 4 KB | 0 | Flash persistence |

### 7.3 Inter-Task Queues

| Queue | Item Type | Producer | Consumer |
|-------|-----------|----------|----------|
| `rx_radio_queue` | `RadioPacket` | radio_task | protocol_task |
| `tx_radio_queue` | `TxRequest` | protocol_task | radio_task |
| `chat_inbound_queue` | `ChatMessage` | protocol_task | chat_task |
| `chat_outbound_queue` | `ChatMessage` | chat_task / websocket | protocol_task |
| `ui_event_queue` | `UiEvent` | keyboard / wifi | display_task |
| `storage_queue` | `StorageOp` | chat_task | storage_task |
| `log_queue` | `LogEntry` | all | serial (optional) |

**Synchronization:** `EventGroupHandle_t` for system ready, WiFi connected, LoRa linked.

---

## 8. Storage Architecture

### 8.1 Configuration (Preferences / NVS)

Namespace: `loralink`

| Key | Type | Default |
|-----|------|---------|
| `device_name` | string | "Cardputer" / "Gateway" |
| `device_id` | uint16 | 0x0001 / 0x0002 |
| `freq_hz` | uint32 | 433125000 |
| `bw_hz` | uint32 | 125000 |
| `sf` | uint8 | 12 |
| `cr` | uint8 | 5 |
| `sync_word` | uint16 | 0x0012 |
| `tx_power` | int8 | 14 |
| `wifi_ssid` | string | board-specific |
| `wifi_pass` | string | generated on first boot |
| `aes_key` | bytes[16] | zeros (encryption off) |
| `aes_enabled` | bool | false |

### 8.2 Message Storage (LittleFS) — Both Devices

- **Max messages:** 1000 on Cardputer **and** Gateway
- **Structure:** Circular buffer index in NVS + append-only records in `messages.bin`
- **Record:** `{ id, timestamp, source_id, direction, len, utf8_payload }`
- **Writes:** Async via `storage_task` (never block radio)

---

## 9. Security Architecture

### 9.1 Threat Model (v1)

- Passive eavesdropping on 433 MHz → mitigated by optional AES-128
- Unauthorized WiFi access → WPA2 password
- Replay attacks → sequence numbers + dedup window
- No PKI in v1 — shared symmetric key only

### 9.2 Encryption

- Algorithm: **AES-128-CTR** (stream-friendly, no padding issues)
- Key distribution: 6-digit pairing code → PBKDF2-like simple KDF (v1) → `aes_key`
- Encrypted scope: MESSAGE payload only (header remains readable for routing)

---

## 10. Web Application (T-Beam)

Hosted entirely on ESP32 flash (PROGMEM / LittleFS).

```
/wifi AP → 192.168.4.1
  ├── GET  /              → index.html (chat UI)
  ├── GET  /style.css
  ├── GET  /app.js
  ├── WS   /ws            → WebSocket chat
  ├── GET  /api/status    → RSSI, packets, uptime (JSON)
  └── GET  /api/messages  → history (paginated)
```

**WebSocket message format (JSON):**

```json
{ "type": "message", "text": "hello", "ts": 1720320000 }
{ "type": "status", "rssi": -87, "linked": true }
```

**No frameworks.** Vanilla JS with auto-reconnect WebSocket.

---

## 11. Cardputer UI Architecture

### 11.1 Screen Layout (240×135)

```
┌────────────────────────────────────┐
│ ■ LoRa  RSSI:-87  Bat:78%  12:34  │  ← Status bar (16px)
├────────────────────────────────────┤
│                                    │
│  [CP] You: test message            │
│  [GW] Phone: hello from web        │  ← Scrollable chat (95px)
│                                    │
├────────────────────────────────────┤
│ > typing buffer_                   │  ← Input line (20px)
└────────────────────────────────────┘
```

### 11.2 Settings Menu (Fn+S)

- Device name, LoRa params (read-only in v1), WiFi AP toggle, encryption, about

### 11.3 Rendering

- `DisplayRenderer` class with dirty-rectangle optimization
- Dark palette: bg `#0D1117`, text `#E6EDF3`, accent `#58A6FF`
- No full-screen clear unless necessary

---

## 12. Build System Architecture

### 12.1 Monorepo Layout (Phase 2 preview)

```
LoRaLink/
├── platformio.ini              # [env:cardputer], [env:tbeam]
├── src/
│   ├── main_cardputer.cpp
│   ├── main_gateway.cpp
│   └── app/                    # Application entry
├── include/
│   └── loralink/               # Public headers
├── lib/
│   ├── drivers/
│   │   ├── e220/
│   │   └── sx1278/
│   ├── protocol/
│   ├── transport/
│   ├── routing/
│   ├── chat/
│   ├── storage/
│   ├── display/
│   ├── keyboard/
│   ├── wifi/
│   ├── webserver/
│   ├── websocket/
│   ├── crypto/
│   └── common/
└── data/                       # Web assets (Gateway)
    ├── index.html
    ├── style.css
    └── app.js
```

### 12.2 Compile-Time Board Selection

```ini
[env:cardputer]
build_flags = -DBOARD_CARDPUTER -DROLE_TERMINAL

[env:gateway]
build_flags = -DBOARD_T3_V161 -DROLE_GATEWAY
```

Shared library code compiled for both; board-specific files guarded by `#if defined(BOARD_*)`.

---

## 13. Logging & Diagnostics

```cpp
enum class LogLevel { Info, Debug, Warning, Error };

class Logger {
public:
    static void log(LogLevel lvl, const char* tag, const char* fmt, ...);
};
```

**Serial output (115200):**

```
[I][RADIO] E220 init OK, ch=0x17, air=2.4k
[D][PROTO] TX seq=42 type=MESSAGE len=24 crc=0xA3F1
[W][TRANS] ACK timeout seq=42 retry=1/3
[E][RADIO] SX1262 begin failed: ERR_CHIP_NOT_FOUND
```

Packet dumps: hex preview first 32 bytes at DEBUG level.

---

## 14. Boot Sequence

```
Power On
   │
   ▼
HAL Init (GPIO, I2C, SPI/UART)
   │
   ▼
Load Config (Preferences) ──► defaults if missing
   │
   ▼
RadioFactory::create() → initialize() → setConfig()
   │
   ▼
Create FreeRTOS tasks + queues
   │
   ▼
Send HELLO → wait PONG (5s timeout)
   │
   ├── OK  → mark LINK_UP, start chat
   └── FAIL → show "No peer" in UI, retry background
   │
   ▼
[Gateway] Start WiFi AP + WebServer + WebSocket
[Cardputer] Start Display + Keyboard tasks
```

---

## 15. Architectural Decision Records (ADR)

### ADR-001: Monorepo with shared protocol stack

**Decision:** Single repository, two PlatformIO environments.  
**Rationale:** Protocol, transport, and chat layers are identical on both devices. Only drivers and UI differ. Reduces drift.

### ADR-002: Custom binary protocol (not Meshtastic/LoRaWAN)

**Decision:** Clean-room `LoRaLink` packet format.  
**Rationale:** Meshtastic incompatible with E220 UART module. LoRaWAN requires gateways.

### ADR-003: IRadioDriver abstraction

**Decision:** Virtual interface with two implementations.  
**Rationale:** E220 and SX1262 have fundamentally different host interfaces. Application cannot branch on radio type.

### ADR-004: FreeRTOS queues over shared state

**Decision:** All inter-task data via queues; no mutex-protected globals.  
**Rationale:** Predictable latency for radio task; avoids priority inversion on ESP32.

### ADR-005: WebSocket over HTTP polling

**Decision:** WebSocket for phone chat.  
**Rationale:** Lower latency, bi-directional, still hostable on ESP32 (`AsyncWebSocket` or `WebSocketsServer`).

### ADR-006: LittleFS for messages, NVS for config

**Decision:** Split storage by access pattern.  
**Rationale:** NVS for small KV; LittleFS for circular 1000-message log.

### ADR-007: Point-to-point routing only (v1)

**Decision:** No multi-hop mesh in v1. `hop_count`/`ttl` reserved.  
**Rationale:** Two-device system. Mesh adds complexity without current requirement.

### ADR-008: AES-128-CTR optional encryption

**Decision:** Symmetric, off by default.  
**Rationale:** ESP32 has hardware AES; CTR avoids padding on variable-length chat.

---

## 16. Risks & Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| E220 ↔ SX1262 PHY mismatch | No communication | CONFIG_SYNC + documented defaults; lab test in Phase 10 |
| E220 200-byte limit | Fragmentation complexity | Fragmentation layer; max 180-byte wire payload |
| ESP32 RAM pressure | Task crashes | Static queue sizes; pooled buffers; monitor heap |
| WebSocket + LoRa concurrent | Watchdog / WDT | Separate tasks; non-blocking drivers |
| T-Beam PMU not initialized | Radio dead | Board HAL init AXP2101 before radio |
| Cardputer E220 DIP mode | Config fails | Boot-time detection + UI warning |

---

## 17. Phase Roadmap (Remaining)

| Phase | Deliverable | Status |
|-------|-------------|--------|
| **1** | System architecture | ✅ Complete |
| **2** | Directory tree | ✅ Complete |
| 3 | PlatformIO configuration | Pending approval |
| 4 | Interfaces (headers) | Pending |
| 5 | Radio drivers | Pending |
| 6 | Protocol + transport | Pending |
| 7 | Application logic | Pending |
| 8 | Cardputer UI | Pending |
| 9 | Web application | Pending |
| 10 | Integration testing | Pending |

---

## 18. Confirmed Decisions (Phase 2)

| Item | Decision |
|------|----------|
| Gateway board | **LilyGO T3 LoRa32 V1.6.1** + SX1278 433 MHz |
| Cardputer radio | **E220-400T22S** 433 MHz (M5 Unit, Grove UART) |
| Message storage | **Both devices**, 1000-msg circular buffer |
| Driver folder | `lib/drivers/sx1278/` (not sx1262) |

### Open Questions

1. **ESPAsyncWebServer** for WebSocket — acceptable dependency?
2. **Captive portal** — v1 or defer?

---

*Phase 1 updated post-confirmation. See `PHASE2_DIRECTORY.md` for full tree.*

*Awaiting confirmation to proceed to Phase 3: PlatformIO Configuration.*
