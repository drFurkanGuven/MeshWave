# LoRaLink — Phase 8: Cardputer UI

**Status:** Complete — awaiting approval for Phase 9

## Screen layout (240×135)

| Region | Height | Content |
|--------|--------|---------|
| Status bar | 16px | LoRa link, RSSI, battery %, uptime clock |
| Chat area | ~97px | Scrollable `[CP]` / `[GW]` / `[You]` messages |
| Input line | 20px | `> buffer_` with blinking cursor |

Dark theme colors from `lib/display/theme.h` (`#0D1117` background).

## Controls

| Key | Action |
|-----|--------|
| **Enter** | Send message to Gateway |
| **Tab** | Clear input buffer |
| **Backspace/Del** | Delete character |
| **Fn+S** | Toggle settings overlay (read-only v1) |
| Serial | Type line + Enter (fallback) |

## Settings overlay (Fn+S)

Read-only display of device name, ID, LoRa PHY, AES flag, WiFi SSID, firmware version.

## Modules

| File | Role |
|------|------|
| `lib/display/cardputer_ui.cpp` | Dirty-region renderer |
| `lib/keyboard/keyboard_task.cpp` | M5 keyboard polling + Fn combos |
| `src/app/cardputer_app.cpp` | Wires UI, keyboard, chat, status |

## Flash

```bash
pio run -e cardputer -t upload
pio device monitor
```

---

*Phase 9: Gateway web app (ESPAsyncWebServer + WebSocket)*
