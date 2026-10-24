#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <atomic>
#include "profiles.h"
#include "app_state.h"
namespace truck {
class BleProtocol {
 public:
  using CommandFn = void(*)(const String&);
  void begin(CommandFn fn){
    handler_=fn; commands_=xQueueCreate(8,sizeof(Command));
    BLEDevice::init("TruckLoad-Heltec"); BLEDevice::setMTU(247);
    BLEDevice::setCustomGattsHandler([](esp_gatts_cb_event_t event,esp_gatt_if_t,esp_ble_gatts_cb_param_t* p){if(event==ESP_GATTS_START_EVT)Serial.printf("BLE service start status=%d handle=%u\n",p->start.status,p->start.service_handle);});
    server_=BLEDevice::createServer(); server_->setCallbacks(new ServerCb(*this));
    auto*s=server_->createService(SERVICE);
    tx_=s->createCharacteristic(TX,BLECharacteristic::PROPERTY_NOTIFY); tx_->addDescriptor(new BLE2902());
    rx_=s->createCharacteristic(RX,BLECharacteristic::PROPERTY_WRITE|BLECharacteristic::PROPERTY_WRITE_NR);
    rx_->setCallbacks(new RxCb(*this)); bool started=s->start();
    Serial.printf("BLE UART: started=%d service=%u tx=%u rx=%u\n",started,s->getHandle(),tx_->getHandle(),rx_->getHandle());
    if(!started)return;
    esp_ble_gatts_show_local_database();
    auto*a=server_->getAdvertising();a->addServiceUUID(SERVICE);a->start();
  }
  // Process writes on the main loop so calibration, NVS and rendering never race.
  void poll(){if(advertisePending_&&!connected_&&(int32_t)(millis()-advertiseAt_)>=0){advertisePending_=false;server_->startAdvertising();}Command c; if(commands_&&xQueueReceive(commands_,&c,0)==pdTRUE&&handler_)handler_(String(c.text));}
  void usbOnly(bool value){usbOnly_=value;}
  void notify(const String&s){
    Serial.println(s);
    if(usbOnly_||!connected_||!tx_)return;
    String record=s+"\n";
    uint16_t mtu=server_->getPeerMTU(server_->getConnId());
    size_t chunk=mtu>3?mtu-3:20;
    for(size_t i=0;i<record.length()&&connected_;i+=chunk){
      size_t count=min(chunk,record.length()-i);
      tx_->setValue(reinterpret_cast<const uint8_t*>(record.c_str()+i),count);tx_->notify();delay(12);
    }
  }
  bool connected()const{return connected_;}
 private:
  bool usbOnly_=false;
  struct Command{char text[129];}; QueueHandle_t commands_=nullptr;
  static constexpr const char* SERVICE="6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
  static constexpr const char* RX="6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
  static constexpr const char* TX="6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
  BLEServer*server_=nullptr;BLECharacteristic*tx_=nullptr;BLECharacteristic*rx_=nullptr;
  std::atomic<bool> connected_{false},advertisePending_{false};std::atomic<uint32_t> advertiseAt_{0};CommandFn handler_=nullptr;
  class ServerCb:public BLEServerCallbacks{BleProtocol&o_;public:ServerCb(BleProtocol&o):o_(o){}void onConnect(BLEServer*)override{o_.connected_=true;o_.advertisePending_=false;Serial.println("BLE client connected");}void onDisconnect(BLEServer*s)override{o_.connected_=false;if(o_.commands_)xQueueReset(o_.commands_);o_.advertiseAt_=millis()+300;o_.advertisePending_=true;Serial.println("BLE client disconnected");}};
  class RxCb:public BLECharacteristicCallbacks{BleProtocol&o_;public:RxCb(BleProtocol&o):o_(o){}void onWrite(BLECharacteristic*c)override{
    String x=c->getValue();x.trim();if(!o_.commands_||!x.length()||x.length()>128)return;
    Command msg{};x.toCharArray(msg.text,sizeof(msg.text));xQueueSend(o_.commands_,&msg,0);
  }};
};
inline String tok(const String&s,int n){int start=0;for(int i=0;;i++){int e=s.indexOf('|',start);if(i==n)return e<0?s.substring(start):s.substring(start,e);if(e<0)return"";start=e+1;}}
}
