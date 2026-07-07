# drivers/radio/

`IRadioDriver` — the single abstraction boundary for all radio hardware.

| File | Purpose |
|------|---------|
| `i_radio_driver.h` | Virtual interface (initialize, send, receive, sleep, RSSI) |
| `radio_config.h` | `RadioConfig` struct |
| `radio_packet.h` | `RadioPacket`, `RxMetadata` |
| `radio_factory.h` | `RadioFactory::create()` per board |

Application code must include only headers from this folder — never `e220/` or `sx1278/` directly.
