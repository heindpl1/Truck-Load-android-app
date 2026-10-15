import com.truckload.controller.Telemetry;
import java.nio.charset.StandardCharsets;
import java.util.*;
public class TelemetryTest {
 static int checks=0;static void check(boolean ok){checks++;if(!ok)throw new AssertionError("Check "+checks);}
 public static void main(String[] args){
 String wire="SENSOR|ON|READY|0.900\nT|20.4|12000|75.0|16000|Tráiler\nGPS|1|40.0|-90.0|12.5|0|9|110|DRIVING|1|1\n";
 byte[] bytes=wire.getBytes(StandardCharsets.UTF_8);
 for(int split=0;split<=bytes.length;split++){Telemetry m=new Telemetry();Telemetry.Framer f=new Telemetry.Framer();List<String> records=new ArrayList<>();f.accept(Arrays.copyOfRange(bytes,0,split),records::add);f.accept(Arrays.copyOfRange(bytes,split,bytes.length),records::add);check(records.size()==3);for(String r:records)m.apply(r);check(m.hasWeight()&&m.weight==12000&&m.profile.equals("Tráiler"));check(m.gpsFix&&m.satellites==9&&m.distance==110&&m.gpsAuto);}
 Telemetry m=new Telemetry();m.apply("T|nan|-1|-1|0|NONE");check(!m.hasWeight());m.apply("SENSOR|OFF|OFF|nan");check(!m.ready());m.apply("LED_COLOR|LOW|ABCDEF");check(m.colors.get("LOW").equals("ABCDEF"));m.apply("LED_COLOR|LOW|invalid");check(m.colors.get("LOW").equals("ABCDEF"));m.apply("PLIST_BEGIN|1");m.apply("PROFILE|0|Trailer 1|1|3");check(m.profiles.size()==1&&m.profiles.get(0).active&&m.profiles.get(0).points==3);m.apply("STATUS|Trailer 1|20000|22000|3|140|128|GPS_AUTO_ON|PLANT_SET");check(m.target==20000&&m.overload==22000&&m.points==3&&m.workBrightness==128&&m.plant);m.apply("OLED_LAYOUT|3|0|4");check(m.page==3);m.apply("LIGHT_STATE|WORK|PULSE|140|200");check(m.lightMode.equals("WORK")&&m.pattern.equals("PULSE"));m.clearLive();check(!m.hasWeight()&&!m.gpsFix);
 Telemetry.Framer f=new Telemetry.Framer();List<String> records=new ArrayList<>();byte[] huge=new byte[5000];Arrays.fill(huge,(byte)'x');f.accept(huge,records::add);f.accept("\nPONG\n".getBytes(StandardCharsets.UTF_8),records::add);check(records.size()==1&&records.get(0).equals("PONG"));
 System.out.println("PASS: "+checks+" framing and protocol checks");
 }
}
