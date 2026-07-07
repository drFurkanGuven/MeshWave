# LoRaLink — Phase 9: Gateway Web App

**Status:** Complete — awaiting approval for Phase 10

## Endpoints

| Route | Method | Description |
|-------|--------|-------------|
| `/` | GET | `index.html` chat UI (LittleFS) |
| `/style.css` | GET | Stylesheet |
| `/app.js` | GET | WebSocket client |
| `/ws` | WebSocket | Live chat + status |
| `/api/status` | GET | JSON link stats |
| `/api/messages` | GET | JSON message history |

## WebSocket protocol

**Phone → Gateway:**
```json
{ "type": "message", "text": "merhaba" }
```

**Gateway → Phone:**
```json
{ "type": "history", "messages": [ { "id": 1, "ts": 12, "dir": "in", "text": "..." } ] }
{ "type": "message", "id": 2, "ts": 15, "dir": "out", "text": "..." }
{ "type": "status", "linked": true, "rssi": -87, "tx": 10, "rx": 8, "uptime": 120 }
```

## Usage

1. Flash gateway firmware
2. Upload web assets to LittleFS
3. Connect phone to WiFi `LoRaLink-GW` / `loralink123`
4. Open http://192.168.4.1

```bash
pio run -e gateway -t upload
pio run -e gateway -t buildfs && pio run -e gateway -t uploadfs
```

## Modules

| File | Role |
|------|------|
| `lib/webserver/gateway_web.cpp` | ESPAsyncWebServer + AsyncWebSocket |
| `lib/webserver/api_handlers.cpp` | REST JSON builders |
| `data/*` | Static web UI |
| `src/app/gateway_app.cpp` | Wires WiFi + web + chat |

---

*Phase 10: Integration testing (E220 ↔ SX1278 link validation)*
