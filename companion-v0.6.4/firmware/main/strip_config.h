#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "hardware_config.h"
namespace truck {
// Counts are addressable IC groups, not individual LED packages.
struct StripConfig { uint16_t count[2]={120,7}; uint16_t density[2]={60,84}; uint16_t perGroup[2]={3,7}; };
inline bool validStripConfig(const StripConfig& s){return s.count[0]>=1&&s.count[0]<=hw::MAX_ZONE1_PIXELS&&s.count[1]>=1&&s.count[1]<=hw::MAX_ZONE2_PIXELS&&s.density[0]>=1&&s.density[0]<=1000&&s.density[1]>=1&&s.density[1]<=1000&&s.perGroup[0]>=1&&s.perGroup[0]<=30&&s.perGroup[1]>=1&&s.perGroup[1]<=30;}
inline void loadStrips(StripConfig& s){Preferences p;if(!p.begin("truckload",true))return;StripConfig candidate;if(p.getBytesLength("stripCfg1")==sizeof(candidate)&&p.getBytes("stripCfg1",&candidate,sizeof(candidate))==sizeof(candidate)&&validStripConfig(candidate))s=candidate;p.end();}
inline bool saveStrips(const StripConfig& s){if(!validStripConfig(s))return false;Preferences p;if(!p.begin("truckload",false))return false;bool ok=p.putBytes("stripCfg1",&s,sizeof(s))==sizeof(s);p.end();return ok;}
}
