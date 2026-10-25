#pragma once
#include <Preferences.h>
#include "geo_core.h"
namespace truck {
inline void loadGeo(GeoConfig& c){Preferences p;if(!p.begin("truckGeo",true))return;GeoConfig loaded{};if(p.getBytesLength("cfg1")==sizeof(loaded)&&p.getBytes("cfg1",&loaded,sizeof(loaded))==sizeof(loaded)&&geoConfigValid(loaded))c=loaded;p.end();}
inline bool saveGeo(const GeoConfig& c){if(!geoConfigValid(c))return false;Preferences p;if(!p.begin("truckGeo",false))return false;bool ok=p.putBytes("cfg1",&c,sizeof(c))==sizeof(c);p.end();return ok;}
}
