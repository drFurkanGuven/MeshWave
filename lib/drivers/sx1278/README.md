# drivers/sx1278/

SX1278 SPI driver for LilyGO T3 V1.6.1.

Uses LoRa.h or RadioLib internally — hidden behind `IRadioDriver`.

PHY must match E220: SF12, BW125, CR4/5, sync 0x12, 433.125 MHz.

**Gateway build only.**
