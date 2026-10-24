#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <math.h>

namespace truck {
constexpr uint8_t MAX_PROFILES = 12;
constexpr uint8_t MAX_CAL_POINTS = 8;
constexpr uint8_t PROFILE_NAME_LEN = 28;

struct CalibrationPoint { float psi=0; float weightLb=0; };
struct TrailerProfile {
  bool used=false;
  char name[PROFILE_NAME_LEN]{};
  uint8_t pointCount=0;
  CalibrationPoint points[MAX_CAL_POINTS]{};
  float targetWeightLb=0;
  float overloadWeightLb=0;
};

class ProfileStore {
 public:
  void begin() {
    prefs_.begin("truckprof", false);
    if (prefs_.getUInt("ver",0) != VERSION ||
        prefs_.getBytesLength("profiles") != sizeof(profiles_)) {
      memset(profiles_,0,sizeof(profiles_));
      active_=-1; save();
    } else {
      prefs_.getBytes("profiles",profiles_,sizeof(profiles_));
      active_=prefs_.getChar("active",-1);
      validate();
    }
  }
  void end(){ prefs_.end(); }

  TrailerProfile* active() {
    return active_>=0 && active_<MAX_PROFILES && profiles_[active_].used ? &profiles_[active_] : nullptr;
  }
  const TrailerProfile* active() const {
    return active_>=0 && active_<MAX_PROFILES && profiles_[active_].used ? &profiles_[active_] : nullptr;
  }
  int activeIndex() const { return active_; }
  const TrailerProfile& at(uint8_t i) const { return profiles_[i]; }

  int find(const String& n) const {
    String x=n; x.trim();
    for(uint8_t i=0;i<MAX_PROFILES;i++) if(profiles_[i].used && x.equalsIgnoreCase(profiles_[i].name)) return i;
    return -1;
  }
  int create(const String& raw) {
    String n=clean(raw); if(!n.length()) return -1;
    int e=find(n); if(e>=0){active_=e;save();return e;}
    for(uint8_t i=0;i<MAX_PROFILES;i++) if(!profiles_[i].used){
      memset(static_cast<void*>(&profiles_[i]),0,sizeof(TrailerProfile)); profiles_[i].used=true;
      strncpy(profiles_[i].name,n.c_str(),PROFILE_NAME_LEN-1);
      active_=i; save(); return i;
    }
    return -1;
  }
  bool select(const String& n){int i=find(n);if(i<0)return false;active_=i;save();return true;}
  bool remove(const String& n){int i=find(n);if(i<0)return false;memset(static_cast<void*>(&profiles_[i]),0,sizeof(TrailerProfile));if(active_==i)active_=-1;save();return true;}

  bool addPoint(float psi,float lb){
    auto*p=active(); if(!p||p->pointCount>=MAX_CAL_POINTS||!isfinite(psi)||!isfinite(lb)||psi<0||psi>100||lb<0)return false;
    p->points[p->pointCount++]={psi,lb}; sort(*p); save(); return true;
  }
  bool updatePoint(int idx,float psi,float lb){
    auto*p=active();if(!p||idx<0||idx>=p->pointCount||!isfinite(psi)||!isfinite(lb)||psi<0||psi>100||lb<0)return false;
    p->points[idx]={psi,lb};sort(*p);save();return true;
  }
  bool deletePoint(int idx){
    auto*p=active();if(!p||idx<0||idx>=p->pointCount)return false;
    for(int i=idx;i+1<p->pointCount;i++)p->points[i]=p->points[i+1];
    p->pointCount--;save();return true;
  }
  void clearPoints(){auto*p=active();if(p){p->pointCount=0;save();}}
  void setTarget(float v){auto*p=active();if(p&&v>0){p->targetWeightLb=v;if(p->overloadWeightLb<=0)p->overloadWeightLb=v;save();}}
  void setOverload(float v){auto*p=active();if(p&&v>0){p->overloadWeightLb=v;save();}}

  float weightForPsi(float psi) {
    auto*p=active(); if(!p||p->pointCount<2)return NAN; sort(*p);
    uint8_t a=0,b=1;
    if(psi>=p->points[p->pointCount-1].psi){a=p->pointCount-2;b=p->pointCount-1;}
    else if(psi>p->points[0].psi){
      for(uint8_t i=0;i+1<p->pointCount;i++) if(psi>=p->points[i].psi&&psi<=p->points[i+1].psi){a=i;b=i+1;break;}
    }
    float span=p->points[b].psi-p->points[a].psi;
    if(fabsf(span)<0.001f)return p->points[a].weightLb;
    return p->points[a].weightLb+(psi-p->points[a].psi)/span*(p->points[b].weightLb-p->points[a].weightLb);
  }

  void save(){prefs_.putUInt("ver",VERSION);prefs_.putBytes("profiles",profiles_,sizeof(profiles_));prefs_.putChar("active",active_);}
 private:
  static constexpr uint32_t VERSION=1;
  Preferences prefs_;
  TrailerProfile profiles_[MAX_PROFILES]{};
  int8_t active_=-1;
  static String clean(String s){s.trim();s.replace("|","-");if(s.length()>=PROFILE_NAME_LEN)s=s.substring(0,PROFILE_NAME_LEN-1);return s;}
  static void sort(TrailerProfile&p){for(uint8_t i=0;i+1<p.pointCount;i++)for(uint8_t j=i+1;j<p.pointCount;j++)if(p.points[j].psi<p.points[i].psi){auto t=p.points[i];p.points[i]=p.points[j];p.points[j]=t;}}
  void validate(){if(active_<-1||active_>=MAX_PROFILES||(active_>=0&&!profiles_[active_].used))active_=-1;}
};
}


