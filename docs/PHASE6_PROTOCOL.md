# LoRaLink — Phase 6: Protocol & Transport

**Status:** Complete — awaiting approval for Phase 7  
**Scope:** Wire codec, reliable transport, PING/PONG link test.

---

## PacketCodec

Wire format (little-endian):

| Offset | Field |
|--------|-------|
| 0 | Magic `0x4B4C` ("LK") |
| 2 | Version `0x01` |
| 3 | Packet type |
| 4 | Flags |
| 5 | Sequence u16 |
| 7 | Source ID u16 |
| 9 | Dest ID u16 |
| 11 | Hop count |
| 12 | TTL |
| 13 | Timestamp u32 |
| 17 | Payload length u16 |
| 19 | Payload (≤180 B) |
| 19+N | CRC-16/CCITT-FALSE |

## ReliableTransport

| Feature | Implementation |
|---------|----------------|
| ACK | Auto-ACK on `ACK_REQ` packets |
| Retry | Exponential backoff via `AckManager` |
| Dedup | `SequenceManager` per source |
| PING | Auto-reply `PONG` |
| HELLO | Auto-reply HELLO |
| Stats | tx/rx/ack/retries/crc/duplicates |

## Link Test (Phase 6)

| Device | Behavior |
|--------|----------|
| **Gateway** | PING every 5s → Cardputer |
| **Cardputer** | Auto-PONG, polls RX |

Serial monitor (success):
```
[I][TRANS] PONG from 0001 seq=42 RSSI -87
[I][PROBE] link UP pongs=3 ...
```

## Flash & Test

```bash
pio run -e cardputer -t upload
pio run -e gateway -t upload
pio device monitor
```

Power both devices, antennas on, E220 DIP M0/M1=00.

---

*End of Phase 6. Awaiting confirmation for Phase 7: Application.*
