# LoRaLink — Phase 7: Application

**Status:** Complete — awaiting approval for Phase 8

## Components

| Module | Role |
|--------|------|
| `ConfigStore` | NVS persistence (device name, LoRa PHY, WiFi, transport) |
| `MessageStore` | LittleFS circular buffer, 1000 messages **both devices** |
| `ChatService` | Send/receive MESSAGE packets, history + persistence |
| `app_core` | Boots radio → transport → chat → FreeRTOS task |
| `cardputer_app` | M5 keyboard + serial + basic display |
| `gateway_app` | WiFi AP + serial + PING probe |

## Usage

### Cardputer
- Type on keyboard, **Enter** to send to Gateway
- **Tab** clears input
- Serial monitor: type line + Enter (fallback)
- Messages survive reboot (LittleFS)

### Gateway
- WiFi AP: `LoRaLink-GW` / `loralink123` (NVS configurable)
- Serial: type message + Enter → forwards to Cardputer via LoRa
- PING every 5s for link monitoring

## Flash

```bash
pio run -e cardputer -t upload
pio run -e gateway -t upload
pio device monitor
```

## Test chat

1. Power both devices, E220 DIP M0/M1=00
2. Gateway serial: `merhaba` + Enter
3. Cardputer should show message; reply from keyboard

---

*Phase 8: Full Cardputer UI (status bar, dark theme, settings)*
