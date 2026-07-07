# LoRaLink — Phase 5: Radio Drivers

**Status:** Complete — awaiting approval for Phase 6  
**Scope:** Full `E220Driver` and `Sx1278Driver` implementing `IRadioDriver`.

---

## E220Driver (Cardputer)

| Method | Implementation |
|--------|----------------|
| `initialize()` | UART2 @ 9600, M5 `LoRa_E220::Init`, optional `InitLoRaSetting` |
| `setConfig()` | Maps `RadioConfig` → E220 registers (channel, air rate, power) |
| `sendPacket()` | `SendFrame()` transparent P2P, max 200 bytes |
| `receivePacket()` | `RecieveFrame()` with timeout poll loop |
| `sleep/wake` | Documented no-op (DIP/WOR on hardware) |
| `readRSSI()` | Last RX frame RSSI |
| `isChannelFree()` | UART RX buffer empty |

**DIP switches:** M0=OFF M1=OFF for normal use. For `setConfig()`, set M0=ON M1=ON briefly.

**PHY mapping (E220 air rate):**

| BW | Air rate |
|----|----------|
| ≤125 kHz | 2.4 kbps (SF12 class) |
| ≤250 kHz | 9.6 kbps |
| else | 19.2 kbps |

Channel: `410.125 + ch` MHz, default `0x17` = 433.125 MHz.

---

## Sx1278Driver (T3 Gateway)

| Method | Implementation |
|--------|----------------|
| `initialize()` | SPI + `LoRa.begin`, SF/BW/CR/sync/CRC |
| `setConfig()` | Re-applies LoRa.h parameters |
| `sendPacket()` | `beginPacket` / `write` / `endPacket` |
| `receivePacket()` | `parsePacket` loop with timeout |
| `sleep()` | `LoRa.sleep()` |
| `wake()` | `applyChipConfig()` full re-init |
| `readRSSI()` | `packetRssi()` from last RX |
| `isChannelFree()` | No pending `parsePacket()` |

**Default PHY:** 433.125 MHz, SF12, BW125, CR4/5, sync `0x12`, TX 14 dBm.

---

## Thread Safety

Both drivers use FreeRTOS mutex (`xSemaphoreCreateMutex`) around hardware access.

---

## Boot Self-Test

`runRadioSelfTest()` in `lib/drivers/radio/radio_self_test.cpp`  
Called from `main_cardputer.cpp` / `main_gateway.cpp` on boot.

Expected serial output:
```
[I][RADIO] E220-400T22S ready, RSSI=0
[I][MAIN] Phase 5 driver OK: E220-400T22S
```
or
```
[I][SX1278] RF 433.125 MHz SF12 BW125000 CR4/5 sync=0x12 TX14dBm
[I][MAIN] Phase 5 driver OK: SX1278
```

---

## Interop Checklist (manual)

1. Flash both devices with Phase 5 firmware
2. E220 DIP: M0/M1 = 00
3. Place antennas 1–2 m apart
4. Phase 6 will add PING/PONG over `PacketCodec`

---

*End of Phase 5. Awaiting confirmation to proceed to Phase 6: Protocol.*
