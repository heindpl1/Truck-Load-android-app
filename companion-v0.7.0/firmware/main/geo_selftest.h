#pragma once
#include "geo_core.h"
namespace truck {
inline bool geoSelfTest(int& checks){checks=0;GeoConfig c{};GeoArea a{};a.used=1;strcpy(a.name,"Outer");a.radius=200;a.count=1;a.action[0].mode=1;a.action[1].mode=2;c.areas[0]=a;c.enabled=1;c.dwellMs=2500;
#define GCHECK(x) do{if(!(x))return false;checks++;}while(0)
GCHECK(geoConfigValid(c));GCHECK(GeoEngine::pick(c,true,0,0,GEO_DRIVING)==0);GCHECK(GeoEngine::pick(c,true,.01,0,GEO_DRIVING)==-1);GCHECK(GeoEngine::pick(c,false,0,0,GEO_NO_FIX)==-1);
strcpy(a.name,"Scale");a.radius=20;a.priority=20;c.areas[1]=a;GCHECK(GeoEngine::pick(c,true,0,0,GEO_DRIVING)==1);c.areas[1].priority=10;GCHECK(GeoEngine::pick(c,true,0,0,GEO_DRIVING)==1);c.areas[1].enabled=0;GCHECK(GeoEngine::pick(c,true,0,0,GEO_DRIVING)==0);c.areas[1].enabled=1;c.areas[1].action[0].mode=4;GCHECK(GeoEngine::pick(c,true,0,0,GEO_DRIVING)==0);GCHECK(GeoEngine::pick(c,true,0,0,GEO_STOPPED)==1);
GeoArea poly{};poly.used=1;poly.shape=1;poly.count=4;poly.priority=30;strcpy(poly.name,"Delivery");poly.action[0].mode=2;poly.action[1].mode=2;poly.points[0]={-1000,-1000};poly.points[1]={-1000,1000};poly.points[2]={1000,1000};poly.points[3]={1000,-1000};GCHECK(geoAreaValid(poly));GCHECK(geoContains(poly,0,0));GCHECK(geoContains(poly,.0001,0));GCHECK(!geoContains(poly,.001,0));c.areas[2]=poly;GCHECK(GeoEngine::pick(c,true,0,0,GEO_DRIVING)==2);auto temp=poly.points[1];poly.points[1]=poly.points[2];poly.points[2]=temp;GCHECK(!geoAreaValid(poly));
GeoArea u=poly;u.count=8;GeoPoint outline[]={{0,0},{0,3000},{3000,3000},{3000,2000},{1000,2000},{1000,1000},{3000,1000},{3000,0}};for(int i=0;i<8;i++)u.points[i]=outline[i];GCHECK(geoAreaValid(u));GCHECK(geoContains(u,.0002,.00005));GCHECK(!geoContains(u,.0002,.00015));
GeoEngine engine;auto r=engine.update(c,true,0,0,8,100);GCHECK(r.pending&&!r.active);r=engine.update(c,true,0,0,8,2600);GCHECK(r.id==2&&r.active&&r.motion==GEO_DRIVING);r=engine.update(c,true,0,0,2,2700);GCHECK(r.motion==GEO_DRIVING);r=engine.update(c,true,0,0,0,2800);GCHECK(r.pending&&r.motion==GEO_DRIVING);r=engine.update(c,true,0,0,0,5300);GCHECK(r.motion==GEO_STOPPED);r=engine.update(c,false,0,0,NAN,5400);GCHECK(r.motion==GEO_NO_FIX&&r.id==-1);c.fallback[2].mode=3;r=engine.update(c,false,0,0,NAN,5500);GCHECK(r.active&&r.action.mode==3);c.manualOverride=1;r=engine.update(c,false,0,0,NAN,5600);GCHECK(!r.active);
GCHECK(fabs(groundCoverage(0,.1,1,.2)-1)<.0001);GCHECK(fabs(groundCoverage(.2,.1,1,.2))<.0001);GCHECK(fabs(groundCoverage(.95,.1,1,.2)-.5)<.0001);GCHECK(fabs(groundCoverage(-.05,.1,1,.2)-.5)<.0001);GCHECK(fabs(groundCoverage(1,.1,1,.2)-groundCoverage(0,.1,1,.2))<.0001);
#undef GCHECK
return true;}
}
