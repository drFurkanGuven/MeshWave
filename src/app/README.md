# Application Layer

Device-specific application orchestration.

| File | Role |
|------|------|
| `cardputer_app.*` | Terminal: keyboard, display, optional WiFi keyboard |
| `gateway_app.*` | Gateway: WiFi AP, web UI, OLED status |

Both apps share `ChatService`, `ReliableTransport`, and `MessageStore`.
