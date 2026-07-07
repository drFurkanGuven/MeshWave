#pragma once

// E220-433 varsayılanları ile eşleşen LoRa PHY parametreleri.
// E220: 433.125 MHz, kanal 0x17, hava hızı 2.4 kbps (SF12 / BW125).
// LilyGo SX1278 tarafı bu değerlerle yapılandırılmalıdır.

#define LORA_FREQ_HZ        433125000UL
#define LORA_SPREADING      12
#define LORA_BANDWIDTH_HZ   125000UL
#define LORA_CODING_RATE    5      // 4/5
#define LORA_SYNC_WORD      0x12   // özel ağ (SX1278 8-bit)
#define LORA_PREAMBLE_LEN   8
#define LORA_TX_POWER_DBM   14     // T3 V1.6.1 SX1278 max ~14 dBm

// E220 modül ayarları (Cardputer tarafı — M5_LoRa_E220.h ile kullanılır)
#define E220_CHANNEL        0x17   // 433.125 MHz = 410.125 + 23
#define E220_ADDRESS        0x0000
#define E220_UART_BAUD      9600
#define E220_AIR_RATE_2_4K  0      // DATA_RATE_2_4Kbps enum değeri (Cardputer'da set edilir)

// WiFi erişim noktası (her iki cihazda aynı şifre, farklı SSID)
#define WIFI_AP_PASSWORD    "lorabridge123"

#define LILYGO_AP_SSID      "LilyGo-T3-LoRa"
#define CARDPUTER_AP_SSID   "Cardputer-LoRa"
