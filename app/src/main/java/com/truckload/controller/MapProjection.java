package com.truckload.controller;
public final class MapProjection {
 public static double world(int zoom){return 256*Math.pow(2,zoom);}
 public static double x(double lon,int zoom){return (lon+180)/360*world(zoom);}
 public static double y(double lat,int zoom){double clamped=Math.max(-85.05112878,Math.min(85.05112878,lat)),s=Math.sin(Math.toRadians(clamped));return (.5-Math.log((1+s)/(1-s))/(4*Math.PI))*world(zoom);}
 public static double longitude(double x,int zoom){double w=world(zoom);return ((x%w+w)%w)/w*360-180;}
 public static double latitude(double y,int zoom){return Math.toDegrees(Math.atan(Math.sinh(Math.PI*(1-2*y/world(zoom)))));}
 public static double near(double dx,int zoom){double w=world(zoom);return dx-Math.round(dx/w)*w;}
}
