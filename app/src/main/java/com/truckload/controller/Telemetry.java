package com.truckload.controller;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;
import java.util.*;

/** The shared wire protocol. No Android dependency, so framing and parsing can be tested on a PC. */
public final class Telemetry {
 public static final String[] MODES={"LOW","MID","HIGH","FULL","DRIVE","WORK","DRIVE2"};
 public static final String[] COLOR_NAMES={"Normal load","Approaching target","Near target","At / over target","Driving mode","Work lights","Driving secondary color"};
 public static final String[] COLOR_HELP={"Below 75% of target","75–89% of target","90–99% of target","100% of target or more","Used by automatic driving lighting","White uses the strip's RGBW conversion","Used by chase and alternating patterns"};
 public final Map<String,String> colors=new LinkedHashMap<>();
 public final List<Profile> profiles=new ArrayList<>();
 public String firmware="Unavailable (not reported)",benchZone="BOTH",benchColor="MODE";public int rawAdc=-1;
 public final Set<String> capabilities=new HashSet<>();public boolean capsReceived=false;public int protocolVersion=0;
 public boolean extended=false,bench=false,benchSensor=false;public String driveControl="LOAD",drivePattern="SOLID",benchMode="LOAD";public int driveBrightness=140;public double latitude=Double.NaN,longitude=Double.NaN,plantLat=Double.NaN,plantLon=Double.NaN,adcVolts=Double.NaN,sensorVolts=Double.NaN,benchPercent=50,benchPsi=25,benchWeight=Double.NaN,benchEstimate=Double.NaN;
 public String sensor="OFF",profile="NONE",vehicle="NO_FIX",lightMode="LOAD",pattern="BAR";
 public double psi=Double.NaN,weight=Double.NaN,percent=Double.NaN,target=0,overload=0,speed=Double.NaN,distance=Double.NaN;
 public int points=0,page=0,workBrightness=255,satellites=0,radius=250;
 public boolean gpsFix=false,gpsAuto=false,plant=false;
 public Telemetry(){String[] defaults={"00FF00","FFFF00","FFA500","FF0000","000028","FFFFFF","FFA500"};for(int i=0;i<7;i++)colors.put(MODES[i],defaults[i]);}
 public static final class Profile{public final String name;public final int points;public final boolean active;Profile(String n,int p,boolean a){name=n;points=p;active=a;}}
 public static double number(String s){try{return Double.parseDouble(s);}catch(Exception e){return Double.NaN;}}
 private static int integer(String s){double n=number(s);return Double.isNaN(n)||Double.isInfinite(n)?0:(int)n;}
 public boolean ready(){return sensor.equals("READY");}
 public boolean hasWeight(){return ready()&&Double.isFinite(weight)&&weight>=0;}
 public static boolean validHex(String h){return h!=null&&h.matches("[0-9A-Fa-f]{6}");}
 public String apply(String record){
  String[] a=record.split("\\|",-1);String type=a[0];
  try{
   switch(type){
    case "T":if(a.length<6)break;psi=number(a[1]);weight=number(a[2]);percent=number(a[3]);target=number(a[4]);profile=a[5];break;
    case "SENSOR":if(a.length>=3){sensor=a[1].equals("OFF")?"OFF":a[2];adcVolts=a.length>3?number(a[3]):Double.NaN;sensorVolts=a.length>4?number(a[4]):Double.NaN;rawAdc=a.length>5&&number(a[5])>=0&&number(a[5])<=4095?integer(a[5]):-1;}break;
    case "STATUS":if(a.length<9)break;profile=a[1];target=number(a[2]);overload=number(a[3]);points=integer(a[4]);workBrightness=integer(a[6]);gpsAuto=a[7].equals("GPS_AUTO_ON");plant=a[8].equals("PLANT_SET");break;
    case "GPS":if(a.length<11)break;gpsFix=a[1].equals("1");latitude=number(a[2]);longitude=number(a[3]);gpsFix=gpsFix&&Double.isFinite(latitude)&&Double.isFinite(longitude)&&Math.abs(latitude)<=90&&Math.abs(longitude)<=180;speed=number(a[4]);satellites=integer(a[6]);distance=number(a[7]);vehicle=a[8];gpsAuto=a[9].equals("1");plant=a[10].equals("1");break;
    case "GPS_CONFIG":if(a.length>=4){radius=integer(a[1]);plant=a[2].equals("1");gpsAuto=a[3].equals("1");if(a.length>=6){plantLat=number(a[4]);plantLon=number(a[5]);}}break;
    case "FW_INFO":if(a.length>=2&&a[1].matches("[0-9]+\\.[0-9]+\\.[0-9]+"))firmware=a[1];break;
    case "CAPS":if(a.length>2){capsReceived=true;protocolVersion=integer(a[1]);capabilities.clear();capabilities.addAll(Arrays.asList(a[2].split(",")));extended=protocolVersion>=2&&capabilities.contains("DRIVING")&&capabilities.contains("BENCH");}break;
    case "DRIVE_CONFIG":if(a.length>=5){driveControl=a[1];drivePattern=a[2];driveBrightness=Math.max(1,Math.min(255,integer(a[3])));if(validHex(a[4]))colors.put("DRIVE2",a[4].toUpperCase(Locale.US));}break;
    case "BENCH_STATE":if(a.length>=6){bench=a[1].equals("1");benchMode=a[2];benchPercent=number(a[3]);benchPsi=number(a[4]);benchSensor=a[5].equals("1");benchZone=a.length>6&&Arrays.asList("BOTH","1","2").contains(a[6])?a[6]:"BOTH";benchColor=a.length>7?a[7]:"MODE";}break;
    case "BENCH_RESULT":if(a.length>=4){benchWeight=number(a[2]);benchEstimate=number(a[3]);}break;
    case "OLED_LAYOUT":if(a.length>=2)page=Math.max(0,Math.min(3,integer(a[1])));break;
    case "LIGHT_STATE":if(a.length>=5){lightMode=a[1];pattern=a[2];workBrightness=integer(a[4]);}break;
    case "LED_COLOR":if(a.length>=3&&colors.containsKey(a[1])&&validHex(a[2]))colors.put(a[1],a[2].toUpperCase(Locale.US));break;
    case "PLIST_BEGIN":profiles.clear();break;
    case "PROFILE":if(a.length>=5&&profiles.size()<12)profiles.add(new Profile(a[2],integer(a[4]),a[3].equals("1")));break;
   }
  }catch(RuntimeException ignored){/* An incomplete record never breaks the connection or the UI. */}
  return type;
 }
 public void clearLive(){psi=weight=percent=Double.NaN;latitude=longitude=adcVolts=sensorVolts=Double.NaN;bench=false;gpsFix=false;speed=distance=Double.NaN;satellites=0;vehicle="NO_FIX";}
 public interface RecordListener{void record(String line);}
 public static final class Framer{
  private final ByteArrayOutputStream bytes=new ByteArrayOutputStream();private boolean dropping=false;
  public void reset(){bytes.reset();dropping=false;}
  public void accept(byte[] data,RecordListener listener){
   for(byte b:data){
    if(b=='\n'){if(!dropping){String s=new String(bytes.toByteArray(),StandardCharsets.UTF_8).trim();if(!s.isEmpty())listener.record(s);}reset();}
    else if(!dropping){if(bytes.size()>=4096){bytes.reset();dropping=true;}else bytes.write(b);}
   }
  }
 }
}
