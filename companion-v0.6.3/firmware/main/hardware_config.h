#pragma once
#include <Arduino.h>

namespace truck::hw {
constexpr uint8_t PRESSURE_ADC = 4;

// ESP32 -> SN74AHCT125 -> WS2814 data.
constexpr uint8_t LED_ZONE1_DATA = 43;
constexpr uint8_t LED_ZONE2_DATA = 44;

// Heltec WiFi LoRa 32 V4.x GNSS socket / L76K.
constexpr uint8_t GPS_RX = 39;
constexpr uint8_t GPS_TX = 38;
constexpr uint8_t GPS_POWER = 34; // LOW = powered
constexpr uint8_t GPS_RESET = 42; // HIGH = run
constexpr uint32_t GPS_BAUD = 9600;

constexpr uint8_t OLED_RESET = 21;
constexpr uint8_t OLED_SDA = 17;
constexpr uint8_t OLED_SCL = 18;
constexpr uint8_t OLED_VEXT = 36;

// FUSCH 0-100 PSI, 0.5-4.5 V sensor.
// Signal divider: sensor -> 10k -> ADC node -> 22k -> GND.
constexpr float SENSOR_MAX_PSI = 100.0f;
constexpr float SENSOR_MIN_V = 0.5f;
constexpr float SENSOR_MAX_V = 4.5f;
constexpr float DIVIDER_RATIO = 22.0f / 32.0f;

constexpr uint16_t MAX_ZONE1_PIXELS = 240;
constexpr uint16_t MAX_ZONE2_PIXELS = 120;
}
