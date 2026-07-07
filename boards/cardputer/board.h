#pragma once

/**
 * @file board.h
 * @brief M5Stack Cardputer pin map and feature flags for LoRaLink.
 */

#define BOARD_NAME          "Cardputer"

// Grove HY2.0-4P — E220-400T22S UART
#define E220_UART_TX        1
#define E220_UART_RX        2
#define E220_UART_BAUD      9600

// E220-400T22S: M0/M1 controlled via onboard DIP switches (not GPIO).
// Mode 0 (TX/RX): M0=OFF M1=OFF | Mode 3 (config): M0=ON M1=ON

#define HAS_KEYBOARD        1
#define HAS_COLOR_DISPLAY   1
#define HAS_WIFI_AP         1   // optional remote keyboard
#define HAS_OLED            0
#define HAS_MESSAGE_STORE   1

#define DEFAULT_DEVICE_ID   0x0001
#define DEFAULT_DEVICE_NAME "Cardputer"
