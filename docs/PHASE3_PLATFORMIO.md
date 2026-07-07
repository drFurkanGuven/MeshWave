# LoRaLink — Phase 3: PlatformIO Configuration

**Status:** Complete — awaiting approval for Phase 4  
**Decisions applied:**
- WebSocket via **ESPAsyncWebServer** + AsyncTCP
- Captive portal **deferred**

---

## Environments

| Env | Board | Flash | Partition file | Role |
|-----|-------|-------|--------------|------|
| `cardputer` | `esp32-s3-devkitc-1` | 8 MB | `partitions_8mb.csv` | Terminal |
| `gateway` | `ttgo-lora32-v21` | 4 MB | `partitions_4mb.csv` | WiFi + Web |

## Build Commands

```bash
# Cardputer
pio run -e cardputer
pio run -e cardputer -t upload
pio device monitor

# Gateway
pio run -e gateway
pio run -e gateway -t upload

# Flash web UI to LittleFS (gateway, after Phase 9 assets ready)
pio run -e gateway -t buildfs
pio run -e gateway -t uploadfs
```

## Partition Layout

### Gateway (4 MB) — `partitions_4mb.csv`

| Partition | Size | Purpose |
|-----------|------|---------|
| nvs | 24 KB | Config (Preferences) |
| app0 | 1664 KB | Firmware |
| littlefs | 276 KB | Messages (1000) + web assets |

### Cardputer (8 MB) — `partitions_8mb.csv`

| Partition | Size | Purpose |
|-----------|------|---------|
| nvs | 24 KB | Config |
| app0 | 3072 KB | Firmware |
| littlefs | 5 MB | Messages (1000) + headroom |

## Library Dependencies

### Cardputer
- M5Unified, M5Cardputer
- M5-LoRa-E220 (GitHub)

### Gateway
- LoRa (SX1278)
- U8g2 (OLED)
- AsyncTCP + ESPAsyncWebServer

## Build Isolation

| Mechanism | Purpose |
|-----------|---------|
| `build_src_filter` | Excludes wrong `main_*.cpp` per env |
| `lib_ignore` | Excludes wrong radio driver library |
| `build_flags -I boards/...` | Board-specific pin headers |
| `lib_ldf_mode = deep+` | Nested `lib/drivers/*` discovery |

## Default WiFi (Phase 7+)

| Device | SSID | Password |
|--------|------|----------|
| Gateway | `LoRaLink-GW` | NVS / first-boot random |
| Cardputer AP | `LoRaLink-CP` | NVS / first-boot random |

Static IP: `192.168.4.1`

---

*End of Phase 3. Awaiting confirmation to proceed to Phase 4: Interfaces.*
