#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>
#include "hardware_config.h"

namespace truck {
class GpsModule {
 public:
  void begin() {
    pinMode(hw::GPS_POWER, OUTPUT);
    digitalWrite(hw::GPS_POWER, LOW);
    pinMode(hw::GPS_RESET, OUTPUT);
    digitalWrite(hw::GPS_RESET, HIGH);
    serial_.begin(hw::GPS_BAUD, SERIAL_8N1, hw::GPS_RX, hw::GPS_TX);
  }

  void poll() {
    while (serial_.available()) gps_.encode(serial_.read());
  }

  bool fixed() const { return gps_.location.isValid() && gps_.location.age() < 5000; }
  double lat() const { return fixed() ? gps_.location.lat() : NAN; }
  double lon() const { return fixed() ? gps_.location.lng() : NAN; }
  float speedMph() const { return gps_.speed.isValid() ? gps_.speed.mph() : 0.0f; }
  float courseDeg() const { return gps_.course.isValid() ? gps_.course.deg() : NAN; }
  uint32_t satellites() const { return gps_.satellites.isValid() ? gps_.satellites.value() : 0; }
  float hdop() const { return gps_.hdop.isValid() ? gps_.hdop.hdop() : NAN; }

  double distanceMeters(double lat, double lon) const {
    if (!fixed()) return NAN;
    return TinyGPSPlus::distanceBetween(gps_.location.lat(), gps_.location.lng(), lat, lon);
  }

 private:
  HardwareSerial serial_{1};
  mutable TinyGPSPlus gps_;
};
}


