package com.truckload.controller;
/** Small, platform independent safety decisions used by the UI and its tests. */
public final class SessionPolicy {
 public static boolean fresh(long now,long received){return received>0&&now>=received&&now-received<=4000;}
 public static boolean capture(Telemetry m,boolean connected,long now,long sensor,long telemetry){return connected&&!m.bench&&m.ready()&&Double.isFinite(m.psi)&&m.psi>=0&&m.psi<=100&&m.points<8&&fresh(now,sensor)&&fresh(now,telemetry);}
 public static long retryDelay(int attempt){return attempt==0?2000:attempt==1?8000:attempt==2?20000:-1;}
 public static boolean separated(double previous,double current){return !Double.isFinite(previous)||Double.isFinite(current)&&Math.abs(current-previous)>=2;}
 public static String compatibility(Telemetry m){return !m.capsReceived?"Limited features · capabilities not reported":m.extended?m.capabilities.contains("BENCH_ZONES")&&m.capabilities.contains("BENCH_COLOR")?"Compatible · zone tests supported":"Limited features · zone/channel tests need newer firmware":"Limited features · driving/bench update required";}
}
