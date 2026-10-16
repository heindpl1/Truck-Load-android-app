package com.truckload.controller;
import android.annotation.SuppressLint;
import android.bluetooth.*;
import android.bluetooth.le.*;
import android.content.Context;
import android.os.*;
import java.nio.charset.StandardCharsets;
import java.util.*;

/** Owns one foreground BLE connection. GATT operations are serialized on the main looper. */
@SuppressLint("MissingPermission")
public final class BleClient {
 public static final int OFF=0,SCANNING=1,CONNECTING=2,READY=3;
 public static final UUID SERVICE=UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e"),RX=UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e"),TX=UUID.fromString("6e400003-b5a3-f393-e0a9-e50e24dcca9e"),CCCD=UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
 public interface Listener{void state(int phase,String message);void record(String line);void error(String message);}
 public interface Discovery{void found(BluetoothDevice device,String name);void finished();}
 private final Context context;private final BluetoothAdapter adapter;private final Listener listener;
 private final Handler main=new Handler(Looper.getMainLooper());private final Telemetry.Framer framer=new Telemetry.Framer();
 private final ArrayDeque<String> queue=new ArrayDeque<>();
 private BluetoothGatt gatt;private BluetoothGattCharacteristic receiver;private ScanCallback scanCallback;
 private boolean ready=false,busy=false,discovering=false;private int mtu=23;
 private Runnable scanTimeout,connectTimeout,writeTimeout,mtuTimeout;
 public BleClient(Context c,Listener l){context=c.getApplicationContext();listener=l;BluetoothManager m=(BluetoothManager)c.getSystemService(Context.BLUETOOTH_SERVICE);adapter=m==null?null:m.getAdapter();}
 public String address(){return gatt==null?null:gatt.getDevice().getAddress();}
 public void reconnect(String address){try{connect(adapter.getRemoteDevice(address));}catch(RuntimeException e){fail("Saved controller unavailable. Use Connect to choose it again.");}}
 public boolean available(){return adapter!=null;}
 public boolean enabled(){return adapter!=null&&adapter.isEnabled();}
 public void scan(Discovery discovery){
  stopScan();if(!enabled()){listener.error("Turn Bluetooth on before connecting.");return;}
  BluetoothLeScanner scanner=adapter.getBluetoothLeScanner();if(scanner==null){listener.error("Bluetooth is not ready. Try again.");return;}
  listener.state(SCANNING,"Looking for your Heltec…");
  scanCallback=new ScanCallback(){
   @Override public void onScanResult(int kind,ScanResult result){main.post(()->{if(scanCallback==null)return;String name=result.getScanRecord()==null?null:result.getScanRecord().getDeviceName();if(name==null)name=result.getDevice().getName();discovery.found(result.getDevice(),name==null?"Heltec controller":name);});}
   @Override public void onScanFailed(int code){main.post(()->{stopScan();listener.state(OFF,"Bluetooth scan stopped");listener.error("Could not scan. Check Bluetooth and Nearby devices permission.");discovery.finished();});}
  };
  try{scanner.startScan(Collections.singletonList(new ScanFilter.Builder().setServiceUuid(new ParcelUuid(SERVICE)).build()),new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build(),scanCallback);}
  catch(RuntimeException e){stopScan();listener.state(OFF,"Not connected");listener.error("Could not start Bluetooth scanning.");discovery.finished();return;}
  scanTimeout=()->{stopScan();listener.state(OFF,"Choose a Heltec to connect");discovery.finished();};main.postDelayed(scanTimeout,10000);
 }
 public void stopScan(){if(scanTimeout!=null)main.removeCallbacks(scanTimeout);if(scanCallback!=null&&adapter!=null){try{BluetoothLeScanner scanner=adapter.getBluetoothLeScanner();if(scanner!=null)scanner.stopScan(scanCallback);}catch(RuntimeException ignored){}scanCallback=null;}}
 public void connect(BluetoothDevice device){
  stopScan();closeGatt();listener.state(CONNECTING,"Connecting to the Heltec…");
  try{gatt=device.connectGatt(context,false,callback,BluetoothDevice.TRANSPORT_LE);if(gatt==null)throw new IllegalStateException();}
  catch(RuntimeException e){fail("Could not connect. Keep the Heltec nearby and try again.");return;}
  connectTimeout=()->fail("Connection timed out. Disconnect the web app, then try again.");main.postDelayed(connectTimeout,20000);
 }
 public void disconnect(){stopScan();closeGatt();listener.state(OFF,"Not connected");}
 public void close(){stopScan();closeGatt();main.removeCallbacksAndMessages(null);}
 private void closeGatt(){
  ready=false;busy=false;discovering=false;receiver=null;queue.clear();framer.reset();mtu=23;
  if(connectTimeout!=null)main.removeCallbacks(connectTimeout);if(writeTimeout!=null)main.removeCallbacks(writeTimeout);if(mtuTimeout!=null)main.removeCallbacks(mtuTimeout);
  BluetoothGatt old=gatt;gatt=null;if(old!=null){try{old.disconnect();old.close();}catch(RuntimeException ignored){}}
 }
 private void fail(String message){closeGatt();listener.state(OFF,"Not connected");listener.error(message);}
 private void discover(BluetoothGatt g){if(g!=gatt||discovering)return;discovering=true;if(mtuTimeout!=null)main.removeCallbacks(mtuTimeout);if(!g.discoverServices())fail("Could not read the Heltec's services. Reconnect and try again.");}
 public void send(String command){
  if(!ready||gatt==null||receiver==null){listener.error("Connect the Heltec first.");return;}
  if(command.indexOf('\n')>=0||command.indexOf('\r')>=0){listener.error("A setting contains an unsupported character.");return;}
  if(queue.size()>=32){listener.error("Please wait for the current settings to finish.");return;}
  queue.add(command);writeNext();
 }
 private void writeNext(){
  if(busy||queue.isEmpty()||!ready||gatt==null)return;
  byte[] value=(queue.removeFirst()+"\n").getBytes(StandardCharsets.UTF_8);
  if(value.length>128||value.length>mtu-3){queue.clear();listener.error("Bluetooth packet size is too small. Reconnect before changing this setting.");return;}
  busy=true;boolean started;
  try{
   if(Build.VERSION.SDK_INT>=33)started=gatt.writeCharacteristic(receiver,value,BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)==BluetoothStatusCodes.SUCCESS;
   else{receiver.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);receiver.setValue(value);started=gatt.writeCharacteristic(receiver);}
  }catch(RuntimeException e){started=false;}
  if(!started){busy=false;queue.clear();listener.error("Could not send the setting. Reconnect and try again.");return;}
  writeTimeout=()->fail("The Heltec stopped responding. Reconnect and try again.");main.postDelayed(writeTimeout,5000);
 }
 private final BluetoothGattCallback callback=new BluetoothGattCallback(){
  @Override public void onConnectionStateChange(BluetoothGatt g,int status,int newState){main.post(()->{
   if(g!=gatt)return;
   if(status!=BluetoothGatt.GATT_SUCCESS||newState==BluetoothProfile.STATE_DISCONNECTED){closeGatt();listener.state(OFF,"Heltec disconnected");return;}
   if(newState==BluetoothProfile.STATE_CONNECTED){listener.state(CONNECTING,"Preparing Bluetooth…");if(!g.requestMtu(247))discover(g);else{mtuTimeout=()->discover(g);main.postDelayed(mtuTimeout,2500);}}
  });}
  @Override public void onMtuChanged(BluetoothGatt g,int value,int status){main.post(()->{if(g!=gatt)return;if(status==BluetoothGatt.GATT_SUCCESS)mtu=value;discover(g);});}
  @Override public void onServicesDiscovered(BluetoothGatt g,int status){main.post(()->{
   if(g!=gatt)return;BluetoothGattService service=g.getService(SERVICE);
   if(status!=BluetoothGatt.GATT_SUCCESS||service==null){fail("Truckload service not found. Check the Heltec firmware and reconnect.");return;}
   receiver=service.getCharacteristic(RX);BluetoothGattCharacteristic notify=service.getCharacteristic(TX);
   if(receiver==null||notify==null){fail("The Heltec is missing its control service. Reconnect and try again.");return;}
   BluetoothGattDescriptor descriptor=notify.getDescriptor(CCCD);
   if(descriptor==null||!g.setCharacteristicNotification(notify,true)){fail("Could not enable live readings.");return;}
   boolean ok;
   if(Build.VERSION.SDK_INT>=33)ok=g.writeDescriptor(descriptor,BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)==BluetoothStatusCodes.SUCCESS;
   else{descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);ok=g.writeDescriptor(descriptor);}
   if(!ok)fail("Could not subscribe to the Heltec. Reconnect and try again.");
  });}
  @Override public void onDescriptorWrite(BluetoothGatt g,BluetoothGattDescriptor descriptor,int status){main.post(()->{
   if(g!=gatt)return;if(status!=BluetoothGatt.GATT_SUCCESS){fail("Could not start live readings.");return;}
   if(connectTimeout!=null)main.removeCallbacks(connectTimeout);ready=true;listener.state(READY,"Heltec connected");
  });}
  @Override public void onCharacteristicWrite(BluetoothGatt g,BluetoothGattCharacteristic characteristic,int status){main.post(()->{
   if(g!=gatt)return;if(writeTimeout!=null)main.removeCallbacks(writeTimeout);busy=false;
   if(status!=BluetoothGatt.GATT_SUCCESS){queue.clear();listener.error("The setting could not be sent. Try reconnecting.");}else writeNext();
  });}
  private void received(BluetoothGatt g,byte[] data){byte[] copy=data.clone();main.post(()->{if(g==gatt&&ready)framer.accept(copy,listener::record);});}
  @Override public void onCharacteristicChanged(BluetoothGatt g,BluetoothGattCharacteristic c,byte[] value){if(TX.equals(c.getUuid()))received(g,value);}
  @Override public void onCharacteristicChanged(BluetoothGatt g,BluetoothGattCharacteristic c){if(Build.VERSION.SDK_INT<33&&TX.equals(c.getUuid())){byte[] v=c.getValue();if(v!=null)received(g,v);}}
 };
}
