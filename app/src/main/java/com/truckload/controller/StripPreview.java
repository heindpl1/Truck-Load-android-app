package com.truckload.controller;
import android.content.Context;
import android.graphics.*;
import android.os.SystemClock;
import android.view.View;
/** A clearly labelled phone preview, never a device reading. */
public final class StripPreview extends View {
 private final Telemetry model;private final String loadPattern;private final boolean driving;private final Paint paint=new Paint(3);private final float density;
 private final Runnable tick=new Runnable(){public void run(){invalidate();if(isShown())postDelayed(this,70);}};
 public StripPreview(Context c,Telemetry m,String pattern,boolean drive){super(c);model=m;loadPattern=pattern;driving=drive;density=getResources().getDisplayMetrics().density;setContentDescription("Animated "+(drive?"driving lights":pattern)+" example; preview only");}
 @Override protected void onAttachedToWindow(){super.onAttachedToWindow();post(tick);}
 @Override protected void onDetachedFromWindow(){removeCallbacks(tick);super.onDetachedFromWindow();}
 @Override public void onVisibilityAggregated(boolean visible){super.onVisibilityAggregated(visible);removeCallbacks(tick);if(visible)post(tick);}
 private int rgb(String mode){return Color.parseColor("#"+model.colors.get(mode));}
 private int dim(int c,double level){return Color.rgb((int)(Color.red(c)*level),(int)(Color.green(c)*level),(int)(Color.blue(c)*level));}
 @Override protected void onDraw(Canvas canvas){super.onDraw(canvas);double t=SystemClock.uptimeMillis(),pct=18+(t%9000)/9000*85;String pattern=driving?model.drivePattern:loadPattern;paint.setColor(0xffa2b1c4);paint.setTextSize(11*density);canvas.drawText(driving?pattern+" · preview only":String.format(java.util.Locale.US,"Example · %.0f%% load",pct),0,14*density,paint);int primary=rgb(driving?"DRIVE":pct>=100?"FULL":pct>=90?"HIGH":pct>=75?"MID":"LOW"),second=rgb("DRIVE2");int n=24,lit=(int)Math.round(n*Math.min(100,pct)/100);float gap=3*density,w=(getWidth()-gap*(n-1))/n;for(int zone=0;zone<2;zone++)for(int i=0;i<n;i++){int color=primary;boolean on=true;if(driving){switch(pattern){case "PULSE":color=dim(color,((int)(t/650)%2)==1?1:.18);break;case "CHASE":if((i+n-(int)(t/90)%n)%n>=Math.max(1,n/5))color=second;break;case "ALTERNATE":if((i/6+(int)(t/700)+zone)%2==1)color=second;break;case "BREATHE":color=dim(color,.14+.86*(1+Math.sin(t*2*Math.PI*18/60000))/2);break;case "RAINBOW":color=Color.HSVToColor(new float[]{(float)((i*360f/n+t/35*360/256)%360),1,1});break;}}else{switch(pattern){case "BAR":on=i<lit;break;case "CENTER":on=i>=(n-lit)/2&&i<(n-lit)/2+lit;break;case "PULSE":color=dim(color,.24+.76*(1+Math.sin(t*2*Math.PI*35/60000))/2);break;}}paint.setColor(on?color:0xff26384e);float top=(25+zone*24)*density;canvas.drawRoundRect(i*(w+gap),top,i*(w+gap)+w,top+17*density,2*density,2*density,paint);}}
}
