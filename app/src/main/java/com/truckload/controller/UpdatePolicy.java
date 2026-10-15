package com.truckload.controller;
import java.net.URI;
import java.util.Locale;
/** Validation shared by the updater and its JVM tests. */
public final class UpdatePolicy {
 public static String repository(String value){String s=value.trim();if(s.startsWith("https://github.com/"))s=s.substring(19);if(s.endsWith("/"))s=s.substring(0,s.length()-1);if(s.endsWith(".git"))s=s.substring(0,s.length()-4);if(!s.matches("[A-Za-z0-9][A-Za-z0-9-]{0,38}/[A-Za-z0-9_.-]{1,100}")||s.endsWith("/.")||s.endsWith("/.."))throw new IllegalArgumentException("Enter a GitHub repository, for example owner/repository.");return s;}
 public static String manifestUrl(String repository){return "https://github.com/"+repository(repository)+"/releases/latest/download/truckload-update.json";}
 public static URI https(String value){try{URI u=new URI(value);if(!"https".equalsIgnoreCase(u.getScheme())||u.getHost()==null||u.getUserInfo()!=null||u.getFragment()!=null)throw new Exception();return u;}catch(Exception e){throw new IllegalArgumentException("Updates must use a valid HTTPS download address.");}}
 public static boolean hashMatches(String expected,byte[] digest){if(expected==null||!expected.matches("[a-fA-F0-9]{64}"))return false;StringBuilder h=new StringBuilder();for(byte b:digest)h.append(String.format(Locale.US,"%02x",b&255));return h.toString().equalsIgnoreCase(expected);}
}
