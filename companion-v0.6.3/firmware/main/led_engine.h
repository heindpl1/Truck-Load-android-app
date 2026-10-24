#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "hardware_config.h"
namespace truck {
enum class LightMode:uint8_t{LOAD_BAR,DRIVE_SOLID,WORK_WHITE,OFF};
enum class LoadPattern:uint8_t{BAR,CENTER,PULSE,SOLID};
class LedEngine{
 public:
  void begin(){FastLED.addLeds<WS2812,hw::LED_ZONE1_DATA,RGB>(z1_,hw::MAX_ZONE1_PIXELS).setRgbw(Rgbw(fl::kRGBWDefaultColorTemp,fl::kRGBWExactColors,W0));FastLED.addLeds<WS2812,hw::LED_ZONE2_DATA,RGB>(z2_,hw::MAX_ZONE2_PIXELS).setRgbw(Rgbw(fl::kRGBWDefaultColorTemp,fl::kRGBWExactColors,W0));FastLED.setBrightness(brightness_);clear();}
  void setColor(uint8_t slot,uint32_t value){if(slot<6)colors_[slot]=value&0xffffff;}
  void setDrive(uint8_t pattern,uint32_t secondary){drivePattern_=pattern%6;secondary_=secondary;}
  void setCounts(uint16_t a,uint16_t b){clear();n1_=constrain(a,1,hw::MAX_ZONE1_PIXELS);n2_=constrain(b,1,hw::MAX_ZONE2_PIXELS);}
  void setBrightness(uint8_t b){brightness_=b;FastLED.setBrightness(b);} uint8_t brightness()const{return brightness_;}
  void setMode(LightMode m){mode_=m;}LightMode mode()const{return mode_;}
  void setPattern(LoadPattern p){pattern_=p;}LoadPattern pattern()const{return pattern_;}
  void setBenchOutput(uint8_t zone,int32_t color){benchZone_=zone;benchColor_=color;}
  void render(float pct){
    // Transmit black beyond configured ends, including after a count is reduced.
    fill_solid(z1_,hw::MAX_ZONE1_PIXELS,CRGB::Black);fill_solid(z2_,hw::MAX_ZONE2_PIXELS,CRGB::Black);
    CRGB c=isfinite(pct)?colorFor(pct):CRGB::Black;
    if(mode_==LightMode::WORK_WHITE){fill_solid(z1_,n1_,CRGB(colors_[5]));fill_solid(z2_,n2_,CRGB(colors_[5]));}
    else if(mode_==LightMode::DRIVE_SOLID){drive(z1_,n1_,false);drive(z2_,n2_,true);}
    else if(mode_==LightMode::OFF){fill_solid(z1_,n1_,CRGB::Black);fill_solid(z2_,n2_,CRGB::Black);}
    else if(!isfinite(pct)){fill_solid(z1_,n1_,CRGB::Black);fill_solid(z2_,n2_,CRGB::Black);}
    else{draw(z1_,n1_,pct,c);draw(z2_,n2_,pct,c);}
    if(benchColor_>=0){fill_solid(z1_,n1_,CRGB((uint32_t)benchColor_));fill_solid(z2_,n2_,CRGB((uint32_t)benchColor_));}
    if(benchZone_==2)fill_solid(z1_,hw::MAX_ZONE1_PIXELS,CRGB::Black);
    if(benchZone_==1)fill_solid(z2_,hw::MAX_ZONE2_PIXELS,CRGB::Black);
    FastLED.show();
  }
  void clear(){fill_solid(z1_,hw::MAX_ZONE1_PIXELS,CRGB::Black);fill_solid(z2_,hw::MAX_ZONE2_PIXELS,CRGB::Black);FastLED.show();}
 private:
  CRGB z1_[hw::MAX_ZONE1_PIXELS]{},z2_[hw::MAX_ZONE2_PIXELS]{};uint16_t n1_=120,n2_=60;uint8_t brightness_=140;LightMode mode_=LightMode::LOAD_BAR;LoadPattern pattern_=LoadPattern::BAR;
  uint8_t benchZone_=0;int32_t benchColor_=-1;
  uint8_t drivePattern_=0;uint32_t secondary_=0xffa500;
  uint32_t colors_[6]{0x00ff00,0xffff00,0xffa500,0xff0000,0x000028,0xffffff};
  CRGB colorFor(float p)const{return CRGB(colors_[p>=100?3:(p>=90?2:(p>=75?1:0))]);}
  void drive(CRGB*l,uint16_t n,bool zone2){
    CRGB primary(colors_[4]),second(secondary_);uint32_t t=millis();
    for(uint16_t i=0;i<n;i++){
      CRGB c=primary;
      switch(drivePattern_){
        case 1:c.nscale8_video((t/650)%2?255:45);break;
        case 2:if((i+n-(t/90)%n)%n>=max(1,(int)n/5))c=second;break;
        case 3:if(((i/6+(t/700)+(zone2?1:0))%2))c=second;break;
        case 4:c.nscale8_video(beatsin8(18,35,255));break;
        case 5:c=CHSV((uint8_t)(i*255/max(1,(int)n)+t/35),255,255);break;
      }
      l[i]=c;
    }
  }
  void draw(CRGB*l,uint16_t n,float pct,CRGB c){pct=constrain(pct,0,100);fill_solid(l,n,CRGB::Black);
    if(pattern_==LoadPattern::SOLID){fill_solid(l,n,c);return;}
    uint16_t lit=roundf(n*pct/100.0f);
    if(pattern_==LoadPattern::BAR){for(uint16_t i=0;i<lit;i++)l[i]=c;}
    else if(pattern_==LoadPattern::CENTER){uint16_t start=(n-lit)/2;for(uint16_t k=0;k<lit;k++)l[start+k]=c;}
    else {uint8_t wave=beatsin8(35,60,255);CRGB x=c;x.nscale8_video(wave);fill_solid(l,n,x);}
  }
};
}

