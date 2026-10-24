#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "hardware_config.h"
#include "pressure.h"
#include "vehicle_state.h"
namespace truck {
class OledUi {
 public:
  void begin(){
    pinMode(hw::OLED_VEXT,OUTPUT);digitalWrite(hw::OLED_VEXT,LOW);delay(50);
    pinMode(hw::OLED_RESET,OUTPUT);digitalWrite(hw::OLED_RESET,LOW);delay(20);digitalWrite(hw::OLED_RESET,HIGH);delay(50);
    Wire.begin(hw::OLED_SDA,hw::OLED_SCL);Wire.beginTransmission(0x3c);
    if(Wire.endTransmission()!=0){Serial.println("OLED not detected at 0x3c");return;}
    for(float&v:history_)v=NAN;
    ready_=true;display_.setBusClock(400000);display_.begin();display_.setContrast(200);
    display_.clearBuffer();display_.drawRFrame(0,0,128,64,7);
    display_.setFont(u8g2_font_helvB12_tf);display_.drawStr(10,26,"TRUCK LOAD");
    display_.setFont(u8g2_font_5x7_tf);display_.drawStr(22,42,"v0.6.4 / READY");
    for(int i=0;i<8;i++)display_.drawBox(24+i*10,50,6,3);
    display_.sendBuffer();delay(350);Serial.println("OLED detected at 0x3c");
  }
  void setLayout(uint8_t n){layout_=n%4;pageAt_=millis();}
  void next(){setLayout(layout_+1);}
  void setAuto(bool){auto_=false;}
  bool automatic()const{return auto_;}
  uint8_t layout()const{return layout_;}
  void render(const PressureReading&r,float weight,float pct,const char*profile,float target,uint8_t points,bool gpsFix,uint32_t sats,float mph,VehicleState vs,double plantM,bool bt,bool work){
    if(!ready_)return;
    uint32_t now=millis();
    if(now-historyAt_>=500){historyAt_=now;history_[historyPos_]=r.valid?r.psi:NAN;historyPos_=(historyPos_+1)%48;}
    if(isfinite(pct)){float wanted=constrain(pct,0.0f,100.0f);bar_+=(wanted-bar_)*0.25f;}else bar_=0;
    display_.clearBuffer();header(bt,gpsFix);
    char b[40];
    if(layout_==0){
      display_.setFont(u8g2_font_logisoso18_tf);
      if(isfinite(weight))snprintf(b,sizeof(b),"%.0f",roundf(weight/10.0f)*10.0f);else snprintf(b,sizeof(b),"--");
      display_.drawStr(1,34,b);small();display_.drawStr(113,33,"lb");
      small();if(!r.valid)display_.drawStr(1,41,sensorLabel(r));
      else if(!isfinite(weight))display_.drawStr(1,41,"ADD CALIBRATION");
      else{if(isfinite(pct))snprintf(b,sizeof(b),"%.1f PSI   %.0f%%",r.psi,pct);else snprintf(b,sizeof(b),"%.1f PSI / SET TARGET",r.psi);display_.drawStr(1,41,b);}
      loadBar(0,44,128,8,isfinite(pct));
    }else if(layout_==1){
      display_.setFont(u8g2_font_logisoso18_tf);
      if(r.valid)snprintf(b,sizeof(b),"%.1f",r.psi);else snprintf(b,sizeof(b),"--");
      display_.drawStr(1,34,b);small();display_.drawStr(91,25,"PSI");
      if(isfinite(r.adcVolts))snprintf(b,sizeof(b),"%.2fV",r.adcVolts);else snprintf(b,sizeof(b),"-- V");
      display_.drawStr(91,35,b);chart();
    }else if(layout_==2){
      small();display_.drawStr(1,23,gpsFix?"SATELLITE LOCK":"SEARCHING FOR GPS");
      snprintf(b,sizeof(b),"%lu SAT   %.1f MPH",(unsigned long)sats,mph);display_.drawStr(1,34,b);
      if(isfinite(plantM))snprintf(b,sizeof(b),"PLANT %.0f m",plantM);else snprintf(b,sizeof(b),"PLANT --");
      display_.drawStr(1,45,b);
      int phase=(now/120)%9;for(int i=0;i<9;i++){
        int h=gpsFix?3+(i%4)*2:3+((i==phase)?5:0);display_.drawBox(91+i*4,51-h,2,h);
      }
    }else{
      small();char name[23];snprintf(name,sizeof(name),"%.22s",profile);display_.drawStr(1,23,name);
      if(target>0)snprintf(b,sizeof(b),"TARGET %.0f lb",target);else snprintf(b,sizeof(b),"TARGET --");display_.drawStr(1,34,b);
      snprintf(b,sizeof(b),"CAL %u/8   %s",points,work?"WORK LIGHT":"LOAD MODE");display_.drawStr(1,43,b);
      loadBar(0,46,128,6,isfinite(pct));
    }
    footer(r,vs);display_.sendBuffer();
  }
 private:
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C display_{U8G2_R0,hw::OLED_RESET,hw::OLED_SCL,hw::OLED_SDA};
  bool ready_=false,auto_=false;uint8_t layout_=0,historyPos_=0;
  uint32_t pageAt_=0,historyAt_=0;float bar_=0;
  float history_[48]{};
  void small(){display_.setFont(u8g2_font_5x7_tf);}
  const char* sensorLabel(const PressureReading&r){
    if(!strcmp(r.status,"OFF"))return "SENSOR OFF / BENCH";
    if(!strcmp(r.status,"SETTLING"))return "SENSOR SETTLING";
    if(!strcmp(r.status,"UNSTABLE"))return "SENSOR UNSTABLE";
    return "NO SENSOR SIGNAL";
  }
  void header(bool bt,bool gps){
    const char*titles[]={"LOAD DASHBOARD","PRESSURE TREND","GPS / GEOFENCE","TRAILER PROFILE"};
    small();display_.drawStr(1,8,titles[layout_]);display_.drawStr(91,8,gps?"GPS":"---");
    display_.drawRFrame(115,0,13,10,2);if(bt){display_.drawBox(116,1,11,8);display_.setDrawColor(0);}display_.drawStr(119,8,"B");display_.setDrawColor(1);
    display_.drawHLine(0,12,128);
  }
  void footer(const PressureReading&r,VehicleState vs){
    display_.drawHLine(0,55,128);display_.setFont(u8g2_font_4x6_tf);
    const char*text=!r.valid?r.status:vehicleStateName(vs);display_.drawStr(1,63,text);
    if(auto_)display_.drawStr(75,63,"AUTO");
    for(int i=0;i<4;i++){if(i==layout_)display_.drawDisc(102+i*7,61,2);else display_.drawCircle(102+i*7,61,2);}
  }
  void loadBar(int x,int y,int w,int h,bool valid){
    display_.drawRFrame(x,y,w,h,2);if(!valid)return;
    int fill=roundf((w-4)*bar_/100.0f);if(fill>0)display_.drawBox(x+2,y+2,fill,h-4);
    for(int i=1;i<4;i++){display_.setDrawColor(0);display_.drawVLine(x+w*i/4,y+2,h-4);display_.setDrawColor(1);}
    // A moving cutout follows the real filled bar; missing readings never animate as load.
    int shine=(millis()/90)%(w-4);if(shine<fill){display_.setDrawColor(0);display_.drawVLine(x+2+shine,y+2,h-4);display_.setDrawColor(1);}
  }
  void chart(){
    display_.drawFrame(0,38,128,15);int px=-1,py=0;
    for(int i=0;i<48;i++){
      float p=history_[(historyPos_+i)%48];if(!isfinite(p)){px=-1;continue;}
      int x=2+i*123/47,y=50-roundf(constrain(p,0.0f,100.0f)*10/100.0f);
      if(px>=0){display_.drawLine(px,py,x,y);}
      px=x;py=y;
    }
  }
};
}

