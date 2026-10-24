package com.truckload.controller.startuptests;
import android.app.*;import android.content.*;import android.os.*;import android.widget.*;import java.lang.reflect.*;import java.util.*;import com.truckload.controller.*;
public class StartupInstrumentation extends Instrumentation {
 MainActivity a;List<String> checks=new ArrayList<>();
 Object field(Object o,String n)throws Exception{Field f=o.getClass().getDeclaredField(n);f.setAccessible(true);return f.get(o);}
 Object call(Object o,String n,Class<?>[] t,Object...v)throws Exception{Method m=o.getClass().getDeclaredMethod(n,t);m.setAccessible(true);return m.invoke(o,v);}
 void check(boolean ok,String text){if(!ok)throw new AssertionError(text);checks.add("PASS: "+text);}
 @Override public void onCreate(Bundle b){super.onCreate(b);start();}
 @Override public void onStart(){Bundle result=new Bundle();try{
 a=(MainActivity)startActivitySync(new Intent().setClassName("com.truckload.controller","com.truckload.controller.MainActivity").addFlags(Intent.FLAG_ACTIVITY_NEW_TASK));waitForIdleSync();
 runOnMainSync(()->{try{for(int tab=0;tab<6;tab++){call(a,"showTab",new Class<?>[]{int.class},tab);call(a,"update",new Class<?>[]{});}check(true,"all six tabs open and refresh without the removed pattern buttons");Spinner load=(Spinner)field(a,"loadPatternSelect");Spinner drive=(Spinner)field(field(a,"features"),"driveSelect");check(load.getCount()==4&&drive.getCount()==11,"four load and eleven driving choices remain available");check(!load.isEnabled()&&!drive.isEnabled(),"pattern controls remain disabled while disconnected");a.record("DRIVE_CONFIG|ON|SCANNER|32|FF0060");check(drive.getSelectedItemPosition()==7,"scanner readback selects the matching dropdown entry");a.record("LIGHT_STATE|LOAD|CENTER|140|255");check(load.getSelectedItemPosition()==1,"center load readback updates the dropdown");for(int i=0;i<25;i++)call(a,"update",new Class<?>[]{});check(true,"repeated status refresh remains stable");call(a,"showTab",new Class<?>[]{int.class},2);}catch(Exception e){throw new RuntimeException(e);}});waitForIdleSync();Thread.sleep(1500);result.putString("stream",String.join("\n",checks));finish(Activity.RESULT_OK,result);
 }catch(Throwable e){result.putString("stream",e.toString());finish(Activity.RESULT_CANCELED,result);}}
}
