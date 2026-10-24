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
  void setDrive(uint8_t pattern,uint32_t secondary){drivePattern_=pattern%11;secondary_=secondary;}
  void setCounts(uint16_t a,uint16_t b){clear();n1_=constrain(a,1,hw::MAX_ZONE1_PIXELS);n2_=constrain(b,1,hw::MAX_ZONE2_PIXELS);}
  void setBrightness(uint8_t b){brightness_=b;FastLED.setBrightness(b);} uint8_t brightness()const{return brightness_;}
  void setMode(LightMode m){mode_=m;}LightMode mode()const{return mode_;}
  void setPattern(LoadPattern p){pattern_=p;}LoadPattern pattern()const{return pattern_;}
  void setBenchOutput(uint8_t zone,int32_t color){benchZone_=zone;benchColor_=color;}
  void render(float pct){
    uint32_t now=millis();float dt=lastRender_?min((uint32_t)500,now-lastRender_):0;lastRender_=now;
    if(!isfinite(pct))smoothPct_=NAN;
    else if(!isfinite(smoothPct_))smoothPct_=constrain(pct,0,100);
    else {float target=constrain(pct,0,100);smoothPct_+=(target-smoothPct_)*(1.0f-expf(-dt/260.0f));if(fabsf(target-smoothPct_)<0.005f)smoothPct_=target;}

    // Transmit black beyond configured ends, including after a count is reduced.
    fill_solid(z1_,hw::MAX_ZONE1_PIXELS,CRGB::Black);fill_solid(z2_,hw::MAX_ZONE2_PIXELS,CRGB::Black);
    CRGB c=isfinite(pct)?colorFor(pct):CRGB::Black;
    if(mode_==LightMode::WORK_WHITE){fill_solid(z1_,n1_,CRGB(colors_[5]));fill_solid(z2_,n2_,CRGB(colors_[5]));}
    else if(mode_==LightMode::DRIVE_SOLID){drive(z1_,n1_,false);drive(z2_,n2_,true);}
    else if(mode_==LightMode::OFF){fill_solid(z1_,n1_,CRGB::Black);fill_solid(z2_,n2_,CRGB::Black);}
    else if(!isfinite(pct)){fill_solid(z1_,n1_,CRGB::Black);fill_solid(z2_,n2_,CRGB::Black);}
    else{draw(z1_,n1_,smoothPct_,c);draw(z2_,n2_,smoothPct_,c);}
    if(benchColor_>=0){fill_solid(z1_,n1_,CRGB((uint32_t)benchColor_));fill_solid(z2_,n2_,CRGB((uint32_t)benchColor_));}
    if(benchZone_==2)fill_solid(z1_,hw::MAX_ZONE1_PIXELS,CRGB::Black);
    if(benchZone_==1)fill_solid(z2_,hw::MAX_ZONE2_PIXELS,CRGB::Black);
    FastLED.show();
  }
  void clear(){fill_solid(z1_,hw::MAX_ZONE1_PIXELS,CRGB::Black);fill_solid(z2_,hw::MAX_ZONE2_PIXELS,CRGB::Black);FastLED.show();}
 private:
  CRGB z1_[hw::MAX_ZONE1_PIXELS]{},z2_[hw::MAX_ZONE2_PIXELS]{};uint16_t n1_=120,n2_=60;uint8_t brightness_=140;LightMode mode_=LightMode::LOAD_BAR;LoadPattern pattern_=LoadPattern::BAR;
  float smoothPct_=NAN;uint32_t lastRender_=0;
  uint8_t benchZone_=0;int32_t benchColor_=-1;
  uint8_t drivePattern_=0;uint32_t secondary_=0xffa500;
  uint32_t colors_[6]{0x00ff00,0xffff00,0xffa500,0xff0000,0x000028,0xffffff};
  CRGB colorFor(float p)const{return CRGB(colors_[p>=100?3:(p>=90?2:(p>=75?1:0))]);}
  static float unit(float v){return fmaxf(0.0f,fminf(1.0f,v));}
  static CRGB scaled(CRGB c,float level){c.nscale8((uint8_t)roundf(unit(level)*255));return c;}
  static float coverage(float left,float right,uint16_t group){return unit(fminf(right,group+1.0f)-fmaxf(left,(float)group));}
  CRGB chaseFrame(uint16_t i,uint16_t n,uint16_t head,bool comet,CRGB primary,CRGB second){
    uint16_t d=(head+n-i)%n;float width=max(1,(int)n/5);
    if(comet){float tail=max(2.0f,n*0.65f);return blend(scaled(second,0.08f),primary,(uint8_t)roundf(255*powf(unit(1-d/tail),2)));}
    return d<width?primary:second;
  }
  void drive(CRGB*l,uint16_t n,bool zone2){
    CRGB primary(colors_[4]),second(secondary_);uint32_t t=millis();float phase=fmodf(t/180.0f,n);uint16_t head=(uint16_t)phase;uint8_t fade=(uint8_t)((phase-head)*255);
    for(uint16_t i=0;i<n;i++){
      CRGB c=primary;
      switch(drivePattern_){
        case 1:c=scaled(c,0.18f+0.82f*(0.5f+0.5f*sinf(t/420.0f)));break;
        case 2:case 6:c=blend(chaseFrame(i,n,head,drivePattern_==6,primary,second),chaseFrame(i,n,(head+1)%n,drivePattern_==6,primary,second),fade);break;
        case 3:{float mix=0.5f+0.5f*sinf(t/450.0f);if((i/max(1,(int)n/6)+(zone2?1:0))%2)mix=1-mix;c=blend(primary,second,(uint8_t)(mix*255));break;}
        case 4:c.nscale8_video(beatsin8(18,35,255));break;
        case 5:c=CHSV((uint8_t)(i*255/max(1,(int)n)+t/35),255,255);break;
        case 7:{float pos=(0.5f+0.5f*sinf(t/850.0f))*(n-1);c=blend(scaled(second,0.08f),primary,(uint8_t)(255*unit(1-fabsf(i-pos)/1.6f)));break;}
        case 8:{float wave=0.5f+0.5f*sinf(i*6.2831853f/n-t/650.0f);c=blend(second,primary,(uint8_t)(wave*255));break;}
        case 9:{uint8_t hue=(uint8_t)(90+35*sinf(t/2400.0f+i*3.0f/n)+15*sinf(t/1300.0f-i*6.0f/n));c=CHSV(hue,210,(uint8_t)(95+160*(0.5f+0.5f*sinf(t/1100.0f+i*6.0f/n))));break;}
        case 10:{float step=t/260.0f;int frame=(int)step;float a=(i+frame)%3==0?1:0,b=(i+frame+1)%3==0?1:0;c=blend(scaled(second,0.08f),primary,(uint8_t)(255*(a+(b-a)*(step-frame))));break;}
      }
      l[i]=c;
    }
  }
  void draw(CRGB*l,uint16_t n,float pct,CRGB c){pct=constrain(pct,0,100);fill_solid(l,n,CRGB::Black);
    if(pattern_==LoadPattern::SOLID){fill_solid(l,n,c);return;}
    float filled=n*pct/100.0f;
    if(pattern_==LoadPattern::BAR){for(uint16_t i=0;i<n;i++)l[i]=scaled(c,coverage(0,filled,i));}
    else if(pattern_==LoadPattern::CENTER){float left=(n-filled)/2.0f;for(uint16_t i=0;i<n;i++)l[i]=scaled(c,coverage(left,left+filled,i));}
    else {uint8_t wave=beatsin8(35,60,255);CRGB x=c;x.nscale8_video(wave);fill_solid(l,n,x);}
  }

};
}

