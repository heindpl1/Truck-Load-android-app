package com.truckload.controller;
import android.os.SystemClock;
import android.widget.*;
/** A save is confirmed only by the controller acknowledgement. */
final class SaveFeedback {
 final TextView view;boolean dirty,pending;private long sent;private final String confirmation;
 SaveFeedback(MainActivity a,LinearLayout parent,String savedLabel){confirmation=savedLabel;view=a.text("Connect to read settings.",12,0xffa2b1c4);view.setPadding(0,a.dp(6),0,a.dp(6));parent.addView(view);}
 void edited(){if(pending)return;dirty=true;view.setText("Unsaved changes");view.setTextColor(0xffffcc80);}
 void saving(){pending=true;sent=SystemClock.elapsedRealtime();view.setText("Saving… waiting for controller");view.setTextColor(0xffa2b1c4);}
 void confirmed(){pending=dirty=false;view.setText(confirmation);view.setTextColor(0xff54d6b4);}
 void read(){if(!dirty&&!pending){view.setText(confirmation);view.setTextColor(0xffa2b1c4);}}
 void failed(){if(pending){pending=false;dirty=true;view.setText("Not confirmed · check settings and try again");view.setTextColor(0xffffcc80);}}
 void disconnected(){if(pending){pending=false;dirty=true;}view.setText(dirty?"Unsaved changes · reconnect to apply":"Connect to read settings.");view.setTextColor(0xffa2b1c4);}
 void tick(){if(pending&&SystemClock.elapsedRealtime()-sent>6000)failed();}
}
