#pragma once

#define USER_SETUP_LOADED 1
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define ST7789_2_DRIVER

#if defined(ARDUINO_ESP32S3_DEV) || defined(ARDUINO_ADAFRUIT_METRO_ESP32S3) || defined(ARDUINO_METRO_ESP32S3)
  // GeekMagic S3 (ESP32-S3) Setup
  #define TFT_RGB_ORDER 0 // RGB
  #define TFT_MOSI 11
  #define TFT_SCLK 12
  #define TFT_CS   -1
  #define TFT_DC   7
  #define TFT_RST  6
  #define TFT_BL   14
  #define USE_HSPI_PORT
  #define SPI_FREQUENCY       80000000

#elif defined(ARDUINO_ESP32_DEV)
  // GeekMagic SmallTV Pro (Classic ESP32-WROOM) Setup
  #define TFT_RGB_ORDER 1 // BGR
  #define TFT_MOSI 23
  #define TFT_SCLK 18
  #define TFT_DC   2
  #define TFT_RST  4
  #define TFT_BL   25
  // Keep TFT_CS and TFT_MISO undefined for CS hardwired to GND SPI_MODE3
  #define TFT_SPI_MODE  SPI_MODE3
  #define SPI_FREQUENCY 20000000

#else
  #error "Unsupported board target in include/tft_setup.h"
#endif
