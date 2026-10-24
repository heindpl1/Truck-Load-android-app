package com.truckload.controller;
import android.os.Build;
import android.view.View;
import android.view.Gravity;
import android.widget.*;
/** Accessible, locally collapsible section. Toggling it never sends device commands. */
final class Disclosure {
 final LinearLayout body;final Button header;final TextView summary;private final String title;private boolean expanded;
 Disclosure(MainActivity a,LinearLayout parent,String name,String detail){title=name;LinearLayout card=a.card(parent,null,null);header=a.button(name,()->toggle(),false);header.setGravity(Gravity.START|Gravity.CENTER_VERTICAL);header.setTextSize(16);header.setTypeface(null,android.graphics.Typeface.BOLD);card.addView(header,new LinearLayout.LayoutParams(-1,-2));summary=a.text(detail,12,0xffa2b1c4);summary.setPadding(a.dp(3),a.dp(7),a.dp(3),0);summary.setOnClickListener(v->toggle());card.addView(summary);body=a.vertical();body.setPadding(0,a.dp(16),0,0);card.addView(body);setExpanded(false);}
 void toggle(){setExpanded(!expanded);}void setExpanded(boolean value){expanded=value;body.setVisibility(value?View.VISIBLE:View.GONE);header.setText(title+(value?"  −":"  +"));header.setContentDescription(title+(value?", expanded. Collapse section":", collapsed. Expand section"));if(Build.VERSION.SDK_INT>=30)header.setStateDescription(value?"Expanded":"Collapsed");}boolean isExpanded(){return expanded;}void summary(String text){summary.setText(text);}
}
