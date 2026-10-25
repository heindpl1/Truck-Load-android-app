#pragma once
#include <Arduino.h>
#include <Preferences.h>

namespace truck {
struct AppState {
  bool sensorEnabled = false;
  bool oledAuto = false;
  uint8_t oledLayout = 0;
  bool plantSet = false;
  double plantLat = 0;
  double plantLon = 0;
  float plantRadiusM = 250.0f;
  bool gpsAutomation = false;
  float targetWeightLb = 46000.0f;
  float overloadWeightLb = 47000.0f;
  uint8_t workBrightness = 255;
  uint8_t driveBrightness=140, drivePattern=0; bool driveManual=false; uint32_t driveColor2=0xffa500;
  uint32_t ledColors[6] = {0x00ff00,0xffff00,0xffa500,0xff0000,0x000028,0xffffff};
};

inline void loadState(Preferences& p, AppState& s) {
  p.begin("truckload", true);
  s.sensorEnabled = p.getBool("sensorOn", false);
  s.oledAuto = false;
  s.oledLayout = p.getUChar("oledPage", 0)%4;
  s.plantSet = p.getBool("plantSet", false);
  if(p.isKey("plantLat")) s.plantLat = p.getDouble("plantLat", 0);
  if(p.isKey("plantLon")) s.plantLon = p.getDouble("plantLon", 0);
  if(p.isKey("plantRad")) s.plantRadiusM = p.getFloat("plantRad", 250);
  s.gpsAutomation = p.getBool("gpsAuto", false);
  if(p.isKey("target")) s.targetWeightLb = p.getFloat("target", 46000);
  if(p.isKey("overload")) s.overloadWeightLb = p.getFloat("overload", 47000);
  s.workBrightness = p.getUChar("workBri", 255);
  s.driveBrightness=p.getUChar("driveBri",140);s.drivePattern=p.getUChar("drivePat",0)%12;s.driveManual=p.getBool("driveMan",false);s.driveColor2=p.getUInt("driveC2",0xffa500)&0xffffff;
  for(int i=0;i<6;i++){char key[8];snprintf(key,sizeof(key),"color%d",i);s.ledColors[i]=p.getUInt(key,s.ledColors[i])&0xffffff;}
  p.end();
}
inline void saveState(Preferences& p, const AppState& s) {
  p.begin("truckload", false);
  p.putBool("sensorOn", s.sensorEnabled);
  p.putBool("oledAuto", s.oledAuto);
  p.putUChar("oledPage", s.oledLayout);
  p.putBool("plantSet", s.plantSet);
  p.putDouble("plantLat", s.plantLat);
  p.putDouble("plantLon", s.plantLon);
  p.putFloat("plantRad", s.plantRadiusM);
  p.putBool("gpsAuto", s.gpsAutomation);
  p.putFloat("target", s.targetWeightLb);
  p.putFloat("overload", s.overloadWeightLb);
  p.putUChar("workBri", s.workBrightness);
  p.putUChar("driveBri",s.driveBrightness);p.putUChar("drivePat",s.drivePattern);p.putBool("driveMan",s.driveManual);p.putUInt("driveC2",s.driveColor2);
  for(int i=0;i<6;i++){char key[8];snprintf(key,sizeof(key),"color%d",i);p.putUInt(key,s.ledColors[i]);}
  p.end();
}
}
