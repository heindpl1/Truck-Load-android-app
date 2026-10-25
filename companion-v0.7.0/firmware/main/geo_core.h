#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>
namespace truck {
constexpr int GEO_MAX_AREAS=12,GEO_MAX_POINTS=12,GEO_PATTERN_COUNT=12;
enum GeoMotion:uint8_t{GEO_DRIVING=0,GEO_STOPPED=1,GEO_NO_FIX=2};
struct GeoAction{uint8_t mode=4,pattern=0,brightness=140,reserved=0;uint32_t primary=0x000028,secondary=0xffa500;};
struct GeoPoint{int32_t lat=0,lon=0;};
struct GeoArea{uint8_t used=0,enabled=1,priority=10,shape=0,count=0;char name[25]{};uint16_t radius=50;GeoPoint points[GEO_MAX_POINTS]{};GeoAction action[2]{};};
struct GeoConfig{uint32_t magic=0x47523031;uint8_t enabled=0,manualOverride=0,reverse[2]{0,0};uint16_t dwellMs=2500,marginM=5,spacingMm=1500,widthMm=350;float stoppedMph=1,drivingMph=3;int16_t offsetMm[2]{0,0};GeoAction fallback[3]{};GeoArea areas[GEO_MAX_AREAS]{};};
inline double geoWrap(double d){while(d>180)d-=360;while(d< -180)d+=360;return d;}
inline double geoDistance(double a,double b,double c,double d){const double k=0.017453292519943295;double p=(c-a)*k/2,q=geoWrap(d-b)*k/2,h=sin(p)*sin(p)+cos(a*k)*cos(c*k)*sin(q)*sin(q);return 12742000*asin(sqrt(fmax(0.0,fmin(1.0,h))));}
inline void geoXY(const GeoPoint& p,const GeoPoint& origin,double& x,double& y){y=((double)p.lat-origin.lat)*.011119492664455874;x=geoWrap(((double)p.lon-origin.lon)*1e-7)*111194.92664455874*cos(origin.lat*1e-7*.017453292519943295);}
inline double geoCross(double ax,double ay,double bx,double by,double cx,double cy){return (bx-ax)*(cy-ay)-(by-ay)*(cx-ax);}
inline double geoMetric(const GeoArea& a){if(!a.shape)return 3.141592653589793*a.radius*a.radius;double twice=0;for(int i=0,j=a.count-1;i<a.count;j=i++){double x,y,u,v;geoXY(a.points[j],a.points[0],x,y);geoXY(a.points[i],a.points[0],u,v);twice+=x*v-u*y;}return fabs(twice)/2;}
inline bool geoContains(const GeoArea& a,double lat,double lon,double margin=0){if(!a.shape)return geoDistance(lat,lon,a.points[0].lat*1e-7,a.points[0].lon*1e-7)<=a.radius+margin;GeoPoint q{(int32_t)llround(lat*1e7),(int32_t)llround(lon*1e7)};double x,y;geoXY(q,a.points[0],x,y);bool inside=false;double nearest=1e30;for(int i=0,j=a.count-1;i<a.count;j=i++){double ax,ay,bx,by;geoXY(a.points[j],a.points[0],ax,ay);geoXY(a.points[i],a.points[0],bx,by);double vx=bx-ax,vy=by-ay,den=vx*vx+vy*vy,t=den?fmax(0.0,fmin(1.0,((x-ax)*vx+(y-ay)*vy)/den)):0;double dx=x-ax-t*vx,dy=y-ay-t*vy;nearest=fmin(nearest,sqrt(dx*dx+dy*dy));if(((ay>y)!=(by>y))&&x<(bx-ax)*(y-ay)/(by-ay)+ax)inside=!inside;}return inside||nearest<=fmax(.02,margin);}
inline bool geoActionValid(const GeoAction& a){return a.mode<=4&&a.brightness>=1&&(a.primary&0xff000000)==0&&(a.secondary&0xff000000)==0&&(a.mode==0?a.pattern<4:a.pattern<GEO_PATTERN_COUNT);}
inline bool geoAreaValid(const GeoArea& a){if(!a.used)return true;if(a.enabled>1||a.priority>100||a.shape>1||a.count>GEO_MAX_POINTS||!a.name[0]||!memchr(a.name,0,sizeof(a.name))||!geoActionValid(a.action[0])||!geoActionValid(a.action[1]))return false;for(const char*p=a.name;*p;p++)if((unsigned char)*p<32||*p=='|')return false;if(a.shape?(a.count<3):(a.count!=1||a.radius<5||a.radius>5000))return false;for(int i=0;i<a.count;i++){if(a.points[i].lat< -850000000||a.points[i].lat>850000000||a.points[i].lon< -1800000000||a.points[i].lon>1800000000)return false;if(geoDistance(a.points[0].lat*1e-7,a.points[0].lon*1e-7,a.points[i].lat*1e-7,a.points[i].lon*1e-7)>20000)return false;for(int j=0;j<i;j++)if(a.points[i].lat==a.points[j].lat&&a.points[i].lon==a.points[j].lon)return false;}if(a.shape){if(geoMetric(a)<4)return false;for(int i=0;i<a.count;i++)for(int j=i+1;j<a.count;j++){int k=(i+1)%a.count,l=(j+1)%a.count;if(k==j||l==i)continue;double ax,ay,bx,by,cx,cy,dx,dy;geoXY(a.points[i],a.points[0],ax,ay);geoXY(a.points[k],a.points[0],bx,by);geoXY(a.points[j],a.points[0],cx,cy);geoXY(a.points[l],a.points[0],dx,dy);double p=geoCross(ax,ay,bx,by,cx,cy),q=geoCross(ax,ay,bx,by,dx,dy),r=geoCross(cx,cy,dx,dy,ax,ay),s=geoCross(cx,cy,dx,dy,bx,by);if(p*q<=0&&r*s<=0&&fmax(fmin(ax,bx),fmin(cx,dx))<=fmin(fmax(ax,bx),fmax(cx,dx))&&fmax(fmin(ay,by),fmin(cy,dy))<=fmin(fmax(ay,by),fmax(cy,dy)))return false;}}return true;}
inline bool geoConfigValid(const GeoConfig& c){if(c.magic!=0x47523031||c.enabled>1||c.manualOverride>1||c.reverse[0]>1||c.reverse[1]>1||c.dwellMs>10000||c.marginM>50||c.spacingMm<100||c.spacingMm>10000||c.widthMm<20||c.widthMm>c.spacingMm||!std::isfinite(c.stoppedMph)||!std::isfinite(c.drivingMph)||c.stoppedMph<0||c.stoppedMph>20||c.drivingMph<.1||c.offsetMm[0]<-10000||c.offsetMm[0]>10000||c.offsetMm[1]<-10000||c.offsetMm[1]>10000||c.drivingMph<=c.stoppedMph||c.drivingMph>30)return false;for(auto&a:c.fallback)if(!geoActionValid(a))return false;for(auto&a:c.areas)if(!geoAreaValid(a))return false;return true;}
struct GeoSelection{int id=-1;GeoMotion motion=GEO_NO_FIX;GeoAction action{};bool active=false,pending=false;};
class GeoEngine{
 public:
 GeoSelection update(const GeoConfig& c,bool fix,double lat,double lon,float speed,uint32_t now){
  bool valid=fix&&std::isfinite(lat)&&std::isfinite(lon)&&std::isfinite(speed)&&speed>=0;GeoMotion motion=GEO_NO_FIX;
  if(valid){if(motion_==GEO_NO_FIX)motion_=speed>=c.drivingMph?GEO_DRIVING:GEO_STOPPED;else if(speed>=c.drivingMph)motion_=GEO_DRIVING;else if(speed<=c.stoppedMph)motion_=GEO_STOPPED;motion=motion_;}else motion_=GEO_NO_FIX;
  int next=pick(c,valid,lat,lon,motion,current_);int key=(next+2)*3+motion;
  if(!valid){current_=-1;activeMotion_=GEO_NO_FIX;candidate_=key;since_=now;}
  else if(key!=(current_+2)*3+activeMotion_){if(candidate_!=key){candidate_=key;since_=now;}if((uint32_t)(now-since_)>=c.dwellMs){current_=next;activeMotion_=motion;}}
  else{candidate_=key;since_=now;}
  GeoSelection r;r.id=current_;r.motion=activeMotion_;r.pending=valid&&((current_+2)*3+activeMotion_!=key);r.action=current_>=0?c.areas[current_].action[activeMotion_==GEO_DRIVING?0:1]:c.fallback[activeMotion_];r.active=c.enabled&&!c.manualOverride&&r.action.mode!=4;return r;
 }
 static int pick(const GeoConfig& c,bool fix,double lat,double lon,GeoMotion motion,int previous=-1){if(!fix||motion==GEO_NO_FIX)return -1;int best=-1;double size=1e30;for(int i=0;i<GEO_MAX_AREAS;i++){auto&a=c.areas[i];if(!a.used||!a.enabled||a.action[motion==GEO_DRIVING?0:1].mode==4||!geoContains(a,lat,lon,i==previous?c.marginM:0))continue;double metric=geoMetric(a);if(best<0||a.priority>c.areas[best].priority||(a.priority==c.areas[best].priority&&metric<size-.001)){best=i;size=metric;}}return best;}
 void reset(){current_=-1;candidate_=-99;motion_=activeMotion_=GEO_NO_FIX;since_=0;}
 private:int current_=-1,candidate_=-99;GeoMotion motion_=GEO_NO_FIX,activeMotion_=GEO_NO_FIX;uint32_t since_=0;
};
inline double groundIntegral(double x,double spacing,double width){double n=floor(x/spacing),f=x-n*spacing;return n*width+fmax(0.0,fmin(width,f));}
inline float groundCoverage(double start,double length,double spacing,double width){return (float)fmax(0.0,fmin(1.0,(groundIntegral(start+length,spacing,width)-groundIntegral(start,spacing,width))/length));}
}
