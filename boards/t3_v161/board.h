#pragma once

/**
 * @file board.h
 * @brief LilyGO T3 LoRa32 V1.6.1 pin map (SX1278 433 MHz).
 */

#define BOARD_NAME          "T3-LoRa32-v1.6.1"

// SX1278 LoRa SPI
#define LORA_CS             18
#define LORA_RST            23
#define LORA_DIO0           26
#define LORA_DIO1           33
#define LORA_DIO2           32
#define LORA_SCK            5
#define LORA_MISO           19
#define LORA_MOSI           27

// SSD1306 OLED (I2C)
#define OLED_SDA            21
#define OLED_SCL            22

#define BOARD_LED           25
#define BATTERY_ADC         35

#define HAS_KEYBOARD        0
#define HAS_COLOR_DISPLAY   0
#define HAS_WIFI_AP         1
#define HAS_OLED            1
#define HAS_MESSAGE_STORE   1

#define DEFAULT_DEVICE_ID   0x0002
#define DEFAULT_DEVICE_NAME "Gateway"
