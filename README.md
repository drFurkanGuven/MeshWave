<div align="center">

```diff
- ================================================================================
+       ~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~
+             #   # #####  ###  #   #     #   #  ###  #   # #####
+             ## ## #      #     #   #     #   # #   # #   # #
+             # # # #####   ###  #####     # # # ##### #   # ####
+             #   # #          # #   #     ## ## #   #  # #  #
+             #   # #####  ###   #   #     #   # #   #  # #  #####
+                        M E S H   ·   W A V E
+       ~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~^~~~
-              by Furkan Guven  ·  github.com/drFurkanGuven
- ================================================================================
```

</div>

<p align="center">
  <strong>Local-first LoRa mesh messaging</strong><br/>
  <em>UART E220 ↔ SPI SX1278 · ESP32 · no cloud · no LoRaWAN</em>
</p>

MeshWave is a lightweight, local-only LoRa messaging stack and phone gateway for ESP32 devices, bridging **UART-based E220** radios and **SPI-based SX1278** radios under one protocol.  
Built for field comms: reliable delivery (ACK/retry/dedup), compact packets, on-device history, and a simple WebSocket web UI—no cloud, no LoRaWAN.

| Device A | Device B |
|----------|----------|
| M5Stack Cardputer + E220-400T22S (433 MHz) | LilyGO T3 LoRa32 V1.6.1 + SX1278 (433 MHz) |

**Status:** Phase 10 ready — flash & run.

## Documentation

- [Phase 1 — Architecture](docs/PHASE1_ARCHITECTURE.md)
- [Phase 2 — Directory Tree](docs/PHASE2_DIRECTORY.md)
- [Phase 3 — PlatformIO](docs/PHASE3_PLATFORMIO.md)
- [Phase 4 — Interfaces](docs/PHASE4_INTERFACES.md)
- [Phase 5 — Drivers](docs/PHASE5_DRIVERS.md)
- [Phase 6 — Protocol](docs/PHASE6_PROTOCOL.md)
- [Phase 7 — Application](docs/PHASE7_APPLICATION.md)
- [Phase 8 — Cardputer UI](docs/PHASE8_CARDPUTER_UI.md)
- [Phase 9 — Gateway Web](docs/PHASE9_GATEWAY_WEB.md)
- [Phase 10 — Integration Test](docs/PHASE10_INTEGRATION_TEST.md)

## Hardware (confirmed)

- **Cardputer:** EBYTE E220-400T22S via Grove UART (GPIO1 TX, GPIO2 RX)
- **Gateway:** LilyGO T3 V1.6.1, SX1278 SPI, SSD1306 OLED
- **Storage:** 1000-message circular buffer on **both** devices

## What you get (today)

- **Cardputer UI**: status bar + chat + input, **Fn+S** settings overlay
- **Gateway**: WiFi AP + phone web chat (HTTP + WebSocket) + serial chat
- **Protocol**: ACK/retry/dedup + periodic PING/PONG probe

## Prerequisites (all OS)

- **Git**
- **Python 3.10+**
- **PlatformIO Core (CLI)** (recommended) or PlatformIO VSCode extension
- A USB cable for each board

### Install PlatformIO (recommended CLI)

```bash
python -m pip install -U platformio
platformio --version
```

> If `pip` points to Python 2 / wrong Python, use `python3 -m pip ...`.

## Quick start (flash & run)

From the repository root:

### 1) Build (optional but recommended)

```bash
pio run -e cardputer
pio run -e gateway
```

### 2) Flash firmware

```bash
pio run -e cardputer -t upload
pio run -e gateway -t upload
```

### 3) Flash Gateway Web UI (LittleFS)

Required for the phone UI at `http://192.168.4.1`:

```bash
pio run -e gateway -t buildfs
pio run -e gateway -t uploadfs
```

### 4) Monitor logs (optional)

```bash
pio device monitor
```

Tip: if you have multiple devices, specify the port:

```bash
pio device list
pio device monitor --port <PORT>
```

## First use

### Cardputer (device A)

- Type a message → **Enter** to send
- **Tab** clears input
- **Fn+S** toggles settings overlay (read-only)

### Gateway (device B)

- Starts a WiFi AP: **`LoRaLink-GW` / `loralink123`**
- Phone UI: open `http://192.168.4.1`
- Serial chat fallback: type a line + Enter in serial monitor

## OS-specific notes

### Windows

- **Driver**: most ESP32 boards show up as **CP210x** or **CH340**.
  - If no COM port appears, install the vendor driver and replug.
- **Serial ports** look like `COM3`, `COM12`, etc.
- If upload fails, try:
  - another USB cable (data-capable)
  - a different USB port (avoid unpowered hubs)
  - lowering upload speed in `platformio.ini` (rare)

### macOS

- Serial ports are usually:
  - `/dev/cu.usbserial-*` (CP210x/CH340)
  - `/dev/cu.wchusbserial*`
- If permission errors occur, unplug/replug and retry (macOS sometimes holds the port).

### Linux

- Serial ports are usually `/dev/ttyUSB0` (CP210x/CH340) or `/dev/ttyACM0`.
- Add your user to the dialout group (then log out/in):

```bash
sudo usermod -aG dialout $USER
```

- If uploads require sudo, it’s almost always permissions / group membership.

## Hardware checklist (common pitfalls)

- **Antenna required** on both radios (433 MHz).
- **E220 DIP switches**:
  - normal TX/RX: **M0=OFF, M1=OFF**
  - config mode: **M0=ON, M1=ON**
- Keep devices close (0–2 m) for the first link test, then increase distance.

## Troubleshooting (fast)

- **Web UI opens but chat doesn’t work**
  - Did you run `uploadfs` for the gateway?
  - Check gateway serial logs for WS connect/disconnect lines.
- **`linked` never becomes true**
  - antennas, frequency/LoRa params, E220 DIP mode
  - try close-range first
- **High retries / CRC errors**
  - RF noise, antenna, power, parameter mismatch
  - note: E220 (LLCC68) ↔ SX1278 interop can be environment-dependent

## Development environments

- Cardputer env: `pio run -e cardputer`
- Gateway env: `pio run -e gateway`

## License

TBD (choose before publishing).

## Build

```bash
# Cardputer
pio run -e cardputer -t upload
pio device monitor

# T3 Gateway
pio run -e gateway -t upload
pio run -e gateway -t buildfs && pio run -e gateway -t uploadfs   # web UI
```

**Status:** Phase 10 ready. See [Phase 10 — Integration Test](docs/PHASE10_INTEGRATION_TEST.md).
