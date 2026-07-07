# storage/

**Enabled on both Cardputer and Gateway.**

- `MessageStore`: circular buffer, max 1000 messages
- LittleFS file `messages.bin` + NVS index
- Async writes via `storage_task`
