import com.truckload.controller.UpdatePolicy;
import java.security.MessageDigest;
public class UpdatePolicyTest {
 static int n=0;
 static void check(boolean ok){n++;if(!ok)throw new AssertionError("Check "+n);}
 static void badRepo(String s){boolean threw=false;try{UpdatePolicy.repository(s);}catch(IllegalArgumentException e){threw=true;}check(threw);}
 static void badUrl(String s){boolean threw=false;try{UpdatePolicy.https(s);}catch(IllegalArgumentException e){threw=true;}check(threw);}
 public static void main(String[] args)throws Exception{
  check(UpdatePolicy.repository("https://github.com/heindpl1/Truck-Load-android-app.git").equals("heindpl1/Truck-Load-android-app"));
  check(UpdatePolicy.manifestUrl("heindpl1/Truck-Load-android-app").equals("https://github.com/heindpl1/Truck-Load-android-app/releases/latest/download/truckload-update.json"));
  check(UpdatePolicy.repository("user/repo/").equals("user/repo"));
  for(String r:new String[]{"","../repo","user/..","user/repo/extra","https://example.com/user/repo","user/repo?x=y","user/repo\nextra"})badRepo(r);
  for(String u:new String[]{"http://github.com/x","file:///tmp/a.apk","https://user:example@github.com/a.apk","https://github.com/a.apk#fragment","not a url"})badUrl(u);
  check(UpdatePolicy.https("https://github.com/user/repo/releases/download/v1/app.apk").getHost().equals("github.com"));
  byte[] hash=MessageDigest.getInstance("SHA-256").digest("hello".getBytes("UTF-8"));
  check(UpdatePolicy.hashMatches("2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824",hash));
  check(!UpdatePolicy.hashMatches("0cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824",hash));
  check(!UpdatePolicy.hashMatches("invalid",hash));
  System.out.println("PASS: "+n+" update policy checks");
 }
}
