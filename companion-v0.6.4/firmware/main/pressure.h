#pragma once
#include <Arduino.h>
#include <math.h>
#include "hardware_config.h"
namespace truck {
struct PressureReading {
  float adcVolts=NAN, sensorVolts=NAN, psi=NAN;
  int rawAdc=-1;
  bool valid=false;
  const char* status="OFF";
};
class PressureMonitor {
 public:
  void reset(){good_=0;filtered_=NAN;}
  PressureReading sample(bool enabled){
    PressureReading r;
    if(!enabled){reset();return r;}
    r.rawAdc=analogRead(hw::PRESSURE_ADC); // Separate 12-bit diagnostic sample, not used for pressure math.
    uint32_t samples[ nine ];
    for(int i=0;i<nine;i++)samples[i]=analogReadMilliVolts(hw::PRESSURE_ADC);
    for(int i=0;i<nine-1;i++)for(int j=i+1;j<nine;j++)if(samples[j]<samples[i]){auto t=samples[i];samples[i]=samples[j];samples[j]=t;}
    r.adcVolts=samples[4]/1000.0f;r.sensorVolts=r.adcVolts/hw::DIVIDER_RATIO;
    if(r.sensorVolts<0.38f||r.sensorVolts>4.62f){reset();r.status="NO_SIGNAL";return r;}
    // Reject noisy bursts; an in-range floating pin cannot prove a sensor exists.
    if(samples[7]-samples[1]>100){reset();r.status="UNSTABLE";return r;}
    float raw=constrain((r.sensorVolts-hw::SENSOR_MIN_V)/(hw::SENSOR_MAX_V-hw::SENSOR_MIN_V)*hw::SENSOR_MAX_PSI,0.0f,hw::SENSOR_MAX_PSI);
    if(isnan(filtered_))filtered_=raw;
    else filtered_+=(fabsf(raw-filtered_)>2.0f?0.35f:0.15f)*(raw-filtered_);
    if(good_<8)++good_;
    r.status=good_>=8?"READY":"SETTLING";
    r.valid=good_>=8;r.psi=r.valid?filtered_:NAN;
    return r;
  }
 private:
  static constexpr int nine=9;
  uint8_t good_=0;float filtered_=NAN;
};
}
