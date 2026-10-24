#include <Arduino.h>
#include <Preferences.h>
#include "hardware_config.h"
#include "pressure.h"
#include "gps_module.h"
#include "led_engine.h"
#include "app_state.h"
#include "strip_config.h"
#include "profiles.h"
#include "vehicle_state.h"
#include "ble_protocol.h"
#include "oled_ui.h"

using namespace truck;
StripConfig strips;
Preferences prefs; AppState state; ProfileStore profiles; GpsModule gps; LedEngine leds; VehicleStateMachine motion; BleProtocol ble; OledUi oled;
PressureMonitor pressure; PressureReading reading;
float psi=NAN, weight=NAN, percent=NAN; bool pressureValid=false;
bool bench=false,benchSensor=false;float benchPercent=50,benchPsi=25;String benchMode="LOAD";uint32_t benchUntil=0;LightMode beforeBench=LightMode::LOAD_BAR;uint8_t benchZone=0;String benchColor="MODE";int32_t benchRgb=-1;
static void stopBench(){if(bench){leds.setMode(beforeBench);leds.setBenchOutput(0,-1);benchZone=0;benchColor="MODE";benchRgb=-1;bench=false;benchSensor=false;pressure.reset();pressureValid=false;reading.valid=false;psi=weight=percent=NAN;}}
static bool benchNumber(const String& x,float low,float high,float& out){char*end=nullptr;out=strtof(x.c_str(),&end);return x.length()&&end&&*end==0&&isfinite(out)&&out>=low&&out<=high;}
static const char* driveNames[]={"SOLID","PULSE","CHASE","ALTERNATE","BREATHE","RAINBOW","COMET","SCANNER","WAVE","AURORA","THEATER"};
static void sendDrive(){char h[7];snprintf(h,sizeof(h),"%06lX",(unsigned long)state.driveColor2);ble.notify("DRIVE_CONFIG|"+String(state.driveManual?"ON":state.gpsAutomation?"AUTO":"LOAD")+"|"+driveNames[state.drivePattern]+"|"+String(state.driveBrightness)+"|"+h);}
static void sendBench(){ble.notify("BENCH_STATE|"+String(bench?1:0)+"|"+benchMode+"|"+String(benchPercent,1)+"|"+String(benchPsi,1)+"|"+String(benchSensor?1:0)+"|"+(benchZone==0?String("BOTH"):String(benchZone))+"|"+benchColor);float w=profiles.weightForPsi(benchPsi);auto*p=profiles.active();float pct=isfinite(w)&&p&&p->targetWeightLb>0?w/p->targetWeightLb*100:NAN;ble.notify("BENCH_RESULT|"+String(benchPsi,1)+"|"+String(w,0)+"|"+String(pct,1));}


static void sendStatus(){
  auto*p=profiles.active();String n=p?p->name:"NONE";
  ble.notify("STATUS|"+n+"|"+String(p?p->targetWeightLb:0,0)+"|"+String(p?p->overloadWeightLb:0,0)+"|"+
    String(p?p->pointCount:0)+"|"+String(leds.brightness())+"|"+String(state.workBrightness)+"|"+
    (state.gpsAutomation?"GPS_AUTO_ON":"GPS_AUTO_OFF")+"|"+(state.plantSet?"PLANT_SET":"PLANT_CLEAR"));
}
static void sendProfiles(){
  int count=0;for(int i=0;i<MAX_PROFILES;i++)if(profiles.at(i).used)count++;
  ble.notify("PLIST_BEGIN|"+String(count));
  for(int i=0;i<MAX_PROFILES;i++){auto&p=profiles.at(i);if(!p.used)continue;ble.notify("PROFILE|"+String(i)+"|"+p.name+"|"+String(i==profiles.activeIndex())+"|"+String(p.pointCount));delay(12);}
  ble.notify("PLIST_END");
}

static const char* colorModes[6]={"LOW","MID","HIGH","FULL","DRIVE","WORK"};
static void sendColors(){for(int i=0;i<6;i++){char h[7];snprintf(h,sizeof(h),"%06lX",(unsigned long)state.ledColors[i]);ble.notify("LED_COLOR|"+String(colorModes[i])+"|"+h);}}
static void sendLights(){
  const char* modes[]={"LOAD","DRIVE","WORK","OFF"};const char* patterns[]={"BAR","CENTER","PULSE","SOLID"};
  ble.notify("LIGHT_STATE|"+String(modes[(int)leds.mode()])+"|"+patterns[(int)leds.pattern()]+"|"+String(leds.brightness())+"|"+String(state.workBrightness));
}

static void sendStrips(){ble.notify("STRIP_CONFIG|"+String(strips.count[0])+"|"+String(strips.count[1])+"|"+String(strips.density[0])+"|"+String(strips.density[1])+"|"+String(strips.perGroup[0])+"|"+String(strips.perGroup[1])+"|"+String(hw::MAX_ZONE1_PIXELS)+"|"+String(hw::MAX_ZONE2_PIXELS));}
static bool stripInteger(const String& x,uint16_t& out){if(!x.length()||x.length()>5)return false;uint32_t v=0;for(unsigned i=0;i<x.length();i++){if(x[i]<'0'||x[i]>'9')return false;v=v*10+x[i]-'0';}if(v>65535)return false;out=v;return true;}
static void command(const String&c){
  String op=tok(c,0);op.toUpperCase();
  if(op=="GET_STRIP_CONFIG"){sendStrips();return;}
  if(op=="SET_STRIP_CONFIG"){
    StripConfig next;uint16_t* values[]={&next.count[0],&next.count[1],&next.density[0],&next.density[1],&next.perGroup[0],&next.perGroup[1]};
    int fields=0;for(unsigned i=0;i<c.length();i++)if(c[i]=='|')fields++;
    bool valid=fields==6;for(int i=0;i<6;i++)if(!stripInteger(tok(c,i+1),*values[i]))valid=false;
    if(!valid||!validStripConfig(next)){ble.notify("ERR|BAD_STRIP_CONFIG");return;}
    if(!saveStrips(next)){ble.notify("ERR|STRIP_SAVE_FAILED");return;}
    strips=next;leds.setCounts(strips.count[0],strips.count[1]);ble.notify("OK|SET_STRIP_CONFIG");sendStrips();return;
  }
  if(op=="GET_GPS_CONFIG"){ble.notify("GPS_CONFIG|"+String(state.plantRadiusM,0)+"|"+String(state.plantSet?1:0)+"|"+String(state.gpsAutomation?1:0)+"|"+String(state.plantLat,6)+"|"+String(state.plantLon,6));return;}
  if(op=="GET_FW_INFO"){ble.notify("FW_INFO|0.6.4|Heltec WiFi LoRa 32 V4.2|43|44");return;}
  if(op=="GET_CAPS"){ble.notify("CAPS|4|DRIVING,BENCH,MAP,BENCH_ZONES,BENCH_COLOR,ADC_RAW,FW_INFO,STRIP_CONFIG,FADE_PATTERNS");return;}
  if(op=="GET_DRIVE_CONFIG"){sendDrive();return;}
  if(op=="DRIVE_CONTROL"){String x=tok(c,1);if(x!="ON"&&x!="LOAD"&&x!="AUTO"){ble.notify("ERR|BAD_MODE");return;}state.driveManual=x=="ON";state.gpsAutomation=x=="AUTO";saveState(prefs,state);if(!bench&&leds.mode()!=LightMode::WORK_WHITE)leds.setMode(state.driveManual?LightMode::DRIVE_SOLID:LightMode::LOAD_BAR);ble.notify("OK|DRIVE_CONTROL");sendDrive();return;}
  if(op=="SET_DRIVE_PATTERN"){String x=tok(c,1);int i;for(i=0;i<11;i++)if(x==driveNames[i])break;if(i==11){ble.notify("ERR|BAD_PATTERN");return;}state.drivePattern=i;leds.setDrive(state.drivePattern,state.driveColor2);saveState(prefs,state);ble.notify("OK|SET_DRIVE_PATTERN");sendDrive();return;}
  if(op=="SET_DRIVE_BRIGHTNESS"){float v;if(!benchNumber(tok(c,1),1,255,v)){ble.notify("ERR|BAD_BRIGHTNESS");return;}state.driveBrightness=(uint8_t)v;saveState(prefs,state);ble.notify("OK|SET_DRIVE_BRIGHTNESS");sendDrive();return;}
  if(op=="SET_DRIVE_COLOR2"){String h=tok(c,1);char*end=nullptr;uint32_t value=strtoul(h.c_str(),&end,16);bool good=h.length()==6;for(int i=0;i<h.length();i++)if(!isxdigit((unsigned char)h[i]))good=false;if(!good||!end||*end){ble.notify("ERR|BAD_COLOR");return;}state.driveColor2=value;leds.setDrive(state.drivePattern,value);saveState(prefs,state);ble.notify("OK|SET_DRIVE_COLOR2");sendDrive();return;}
  if(op=="GET_BENCH_STATE"){sendBench();return;}
  if(op=="BENCH"){String x=tok(c,1);if(x=="OFF")stopBench();else if(x=="ON"){if(!bench){beforeBench=leds.mode();bench=true;benchSensor=false;benchPercent=50;benchPsi=25;benchMode="LOAD";benchZone=0;benchColor="MODE";benchRgb=-1;}benchUntil=millis()+15000;}else{ble.notify("ERR|BAD_MODE");return;}ble.notify("OK|BENCH");sendBench();return;}
  if(op=="BENCH_HEARTBEAT"){if(bench)benchUntil=millis()+15000;return;}
  if(op.startsWith("BENCH_")){
    if(!bench){ble.notify("ERR|BENCH_OFF");return;}String x=tok(c,1);float v;
    if(op=="BENCH_ZONE"){if(x!="BOTH"&&x!="1"&&x!="2"){ble.notify("ERR|BAD_ZONE");return;}benchZone=x=="1"?1:x=="2"?2:0;}
    else if(op=="BENCH_COLOR"){if(x!="OFF"&&x!="RED"&&x!="GREEN"&&x!="BLUE"&&x!="WHITE"){ble.notify("ERR|BAD_COLOR");return;}benchColor=x;benchRgb=x=="RED"?0xff0000:x=="GREEN"?0x00ff00:x=="BLUE"?0x0000ff:x=="WHITE"?0xffffff:0;}
    else if(op=="BENCH_LIGHT"){if(x!="LOAD"&&x!="DRIVE"&&x!="WORK"&&x!="OFF"){ble.notify("ERR|BAD_MODE");return;}benchMode=x;benchRgb=-1;benchColor="MODE";}
    else if(op=="BENCH_LOAD"){if(!benchNumber(x,0,120,v)){ble.notify("ERR|BAD_PERCENT");return;}benchPercent=v;benchMode="LOAD";benchRgb=-1;benchColor="MODE";}
    else if(op=="BENCH_PSI"){if(!benchNumber(x,0,100,v)){ble.notify("ERR|BAD_PRESSURE");return;}benchPsi=v;}
    else if(op=="BENCH_SENSOR"){if(x!="ON"&&x!="OFF"){ble.notify("ERR|BAD_MODE");return;}benchSensor=x=="ON";pressure.reset();}
    else{ble.notify("ERR|UNKNOWN_COMMAND");return;}benchUntil=millis()+15000;ble.notify("OK|"+op);sendBench();return;
  }
  if(op=="GET_LED_COLORS"){sendColors();return;}
  if(op=="GET_LIGHT_STATE"){sendLights();return;}
  if(op=="SET_LED_COLOR"){
    String mode=tok(c,1),hex=tok(c,2);mode.toUpperCase();if(hex.startsWith("#"))hex.remove(0,1);
    int slot=-1;for(int i=0;i<6;i++)if(mode==colorModes[i])slot=i;
    if(slot<0||hex.length()!=6){ble.notify("ERR|BAD_COLOR");return;}
    uint32_t value=0;for(int i=0;i<6;i++){char ch=hex[i];int digit=ch>='0'&&ch<='9'?ch-'0':ch>='a'&&ch<='f'?ch-'a'+10:ch>='A'&&ch<='F'?ch-'A'+10:-1;if(digit<0){ble.notify("ERR|BAD_COLOR");return;}value=(value<<4)|digit;}
    state.ledColors[slot]=value;leds.setColor(slot,value);saveState(prefs,state);ble.notify("OK|SET_LED_COLOR|"+mode);sendColors();return;
  }
  if(op=="PING"){ble.notify("PONG");return;}
  if(op=="GET_STATUS"){sendStatus();return;}
  if(op=="SENSOR_ENABLE"){
    String v=tok(c,1);v.toUpperCase();if(v!="ON"&&v!="OFF"){ble.notify("ERR|BAD_MODE");return;}
    state.sensorEnabled=v=="ON";pressure.reset();reading.valid=false;reading.psi=reading.adcVolts=reading.sensorVolts=NAN;reading.rawAdc=-1;reading.status="OFF";pressureValid=false;psi=weight=percent=NAN;
    saveState(prefs,state);ble.notify("OK|SENSOR_ENABLE|"+v);return;
  }
  if(op=="SET_OLED_AUTO"){state.oledAuto=false;oled.setAuto(false);if(tok(c,1)=="OFF")ble.notify("OK|SET_OLED_AUTO|OFF");else ble.notify("ERR|OLED_MANUAL_ONLY");return;}
  if(op=="SET_OLED_LAYOUT"){state.oledLayout=constrain(tok(c,1).toInt(),0,3);state.oledAuto=false;oled.setAuto(false);oled.setLayout(state.oledLayout);saveState(prefs,state);ble.notify("OK|SET_OLED_LAYOUT|"+String(oled.layout()));return;}
  if(op=="GET_OLED_LAYOUT"){ble.notify("OLED_LAYOUT|"+String(oled.layout())+"|"+String(oled.automatic()?1:0)+"|4");return;}
  if(op=="SET_LED_PATTERN"){String x=tok(c,1);x.toUpperCase();if(x=="BAR")leds.setPattern(LoadPattern::BAR);else if(x=="CENTER")leds.setPattern(LoadPattern::CENTER);else if(x=="PULSE")leds.setPattern(LoadPattern::PULSE);else if(x=="SOLID")leds.setPattern(LoadPattern::SOLID);else{ble.notify("ERR|BAD_PATTERN");return;}ble.notify("OK|SET_LED_PATTERN|"+x);sendLights();return;}
  if(op=="PROFILE_LIST"){sendProfiles();return;}
  if(op=="PROFILE_ADD"){int i=profiles.create(tok(c,1));ble.notify(i>=0?"OK|PROFILE_ADD":"ERR|PROFILE_ADD");sendStatus();return;}
  if(op=="PROFILE_SELECT"){ble.notify(profiles.select(tok(c,1))?"OK|PROFILE_SELECT":"ERR|PROFILE_NOT_FOUND");sendStatus();return;}
  if(op=="PROFILE_DELETE"){ble.notify(profiles.remove(tok(c,1))?"OK|PROFILE_DELETE":"ERR|PROFILE_NOT_FOUND");return;}
  if(op=="CAL_CAPTURE"){if(bench){ble.notify("ERR|EXIT_BENCH_TO_CALIBRATE");return;}if(!pressureValid){ble.notify("ERR|PRESSURE_INVALID");return;}float lb=tok(c,1).toFloat();ble.notify(profiles.addPoint(psi,lb)?"OK|CAL_CAPTURE":"ERR|CAL_CAPTURE");return;}
  if(op=="CAL_ADD"){ble.notify(profiles.addPoint(tok(c,1).toFloat(),tok(c,2).toFloat())?"OK|CAL_ADD":"ERR|CAL_ADD");return;}
  if(op=="CAL_CLEAR"){profiles.clearPoints();ble.notify("OK|CAL_CLEAR");return;}
  if(op=="SET_TARGET"){profiles.setTarget(tok(c,1).toFloat());ble.notify("OK|SET_TARGET");return;}
  if(op=="SET_OVERLOAD"){profiles.setOverload(tok(c,1).toFloat());ble.notify("OK|SET_OVERLOAD");return;}
  if(op=="WORK_LIGHT"){String x=tok(c,1);x.toUpperCase();leds.setMode(x=="ON"?LightMode::WORK_WHITE:LightMode::LOAD_BAR);ble.notify("OK|WORK_LIGHT|"+x);sendLights();return;}
  if(op=="SET_WORK_LIGHT_BRIGHTNESS"){state.workBrightness=constrain(tok(c,1).toInt(),1,255);saveState(prefs,state);ble.notify("OK|SET_WORK_LIGHT_BRIGHTNESS");return;}
  if(op=="SET_PLANT_HERE"){if(gps.fixed()){state.plantSet=true;state.plantLat=gps.lat();state.plantLon=gps.lon();saveState(prefs,state);ble.notify("OK|SET_PLANT_HERE");}else ble.notify("ERR|NO_GPS_FIX");return;}
  if(op=="CLEAR_PLANT"){state.plantSet=false;saveState(prefs,state);ble.notify("OK|CLEAR_PLANT");return;}
  if(op=="SET_PLANT_RADIUS"){float m=tok(c,1).toFloat();if(m>=25&&m<=5000){state.plantRadiusM=m;saveState(prefs,state);ble.notify("OK|SET_PLANT_RADIUS");}else ble.notify("ERR|BAD_RADIUS");return;}
  if(op=="GPS_AUTO"){state.driveManual=false;String x=tok(c,1);x.toUpperCase();state.gpsAutomation=x=="ON";saveState(prefs,state);ble.notify("OK|GPS_AUTO|"+x);sendDrive();return;}
  ble.notify("ERR|UNKNOWN_COMMAND");
}
void setup(){
  Serial.begin(115200);analogReadResolution(12);pinMode(hw::PRESSURE_ADC,INPUT);analogReadMilliVolts(hw::PRESSURE_ADC);analogSetPinAttenuation(hw::PRESSURE_ADC,ADC_11db);
  loadState(prefs,state);loadStrips(strips);profiles.begin();gps.begin();leds.begin();leds.setCounts(strips.count[0],strips.count[1]);for(int i=0;i<6;i++)leds.setColor(i,state.ledColors[i]);leds.setDrive(state.drivePattern,state.driveColor2);oled.begin();oled.setLayout(state.oledLayout);oled.setAuto(state.oledAuto);ble.begin(command);
  Serial.println("Truck Load v0.6.4 driving + supervised bench mode");
}
void loop(){
  // USB commands use the same validation and main-loop ownership as BLE.
  static String usbLine;static bool usbOverflow=false;
  while(Serial.available()){char ch=Serial.read();if(ch=='\n'){if(!usbOverflow){usbLine.trim();if(usbLine.length()){ble.usbOnly(true);command(usbLine);ble.usbOnly(false);}}usbLine="";usbOverflow=false;}else if(ch!='\r'){if(usbLine.length()<128)usbLine+=ch;else usbOverflow=true;}}
  gps.poll();if(bench&&(!ble.connected()||(int32_t)(millis()-benchUntil)>=0))stopBench();ble.poll();
  if(bench&&(!ble.connected()||(int32_t)(millis()-benchUntil)>=0))stopBench();
  static uint32_t sensorAt=0,teleAt=0,ledAt=0,oledAt=0;
  if(millis()-sensorAt>=250){sensorAt=millis();reading=pressure.sample(bench?benchSensor:state.sensorEnabled);pressureValid=reading.valid;psi=reading.psi;weight=pressureValid?profiles.weightForPsi(psi):NAN;auto*p=profiles.active();percent=(!isnan(weight)&&p&&p->targetWeightLb>0)?weight/p->targetWeightLb*100.0f:NAN;}
  double plantM=(state.plantSet&&gps.fixed())?gps.distanceMeters(state.plantLat,state.plantLon):NAN;
  auto vs=motion.update(gps.fixed(),gps.speedMph(),plantM,state.plantSet,state.plantRadiusM);
  if(bench){leds.setMode(benchMode=="DRIVE"?LightMode::DRIVE_SOLID:benchMode=="WORK"?LightMode::WORK_WHITE:benchMode=="OFF"?LightMode::OFF:LightMode::LOAD_BAR);}
  else if(leds.mode()!=LightMode::WORK_WHITE){leds.setMode((state.driveManual||(state.gpsAutomation&&vs==VehicleState::DRIVING))?LightMode::DRIVE_SOLID:LightMode::LOAD_BAR);}
  if(millis()-ledAt>=25){ledAt=millis();leds.setBrightness(leds.mode()==LightMode::WORK_WHITE?state.workBrightness:leds.mode()==LightMode::DRIVE_SOLID?state.driveBrightness:140);leds.setBenchOutput(bench?benchZone:0,bench?benchRgb:-1);leds.render(bench?benchPercent:percent);}
  if(millis()-oledAt>=100){oledAt=millis();auto*p=profiles.active();oled.render(reading,weight,percent,p?p->name:"NO PROFILE",p?p->targetWeightLb:0,p?p->pointCount:0,gps.fixed(),gps.satellites(),gps.speedMph(),vs,plantM,ble.connected(),leds.mode()==LightMode::WORK_WHITE);}
  if(millis()-teleAt>=1000){teleAt=millis();String pn=profiles.active()?profiles.active()->name:"NONE";
    sendLights();sendBench();
    ble.notify("SENSOR|"+String((bench?benchSensor:state.sensorEnabled)?"ON":"OFF")+"|"+reading.status+"|"+String(reading.adcVolts,3)+"|"+String(reading.sensorVolts,3)+"|"+String(reading.rawAdc));
    ble.notify("T|"+String(psi,1)+"|"+(isnan(weight)?String("-1"):String(weight,0))+"|"+(isnan(percent)?String("-1"):String(percent,1))+"|"+String(profiles.active()?profiles.active()->targetWeightLb:0,0)+"|"+pn);
    ble.notify("GPS|"+String(gps.fixed()?1:0)+"|"+String(gps.lat(),6)+"|"+String(gps.lon(),6)+"|"+String(gps.speedMph(),1)+"|"+String(gps.courseDeg(),1)+"|"+String(gps.satellites())+"|"+String(plantM,0)+"|"+vehicleStateName(vs)+"|"+String(state.gpsAutomation?1:0)+"|"+String(state.plantSet?1:0));
  }
  delay(2);
}



