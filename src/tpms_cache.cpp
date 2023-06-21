/*
   Based on Neil Kolban example for IDF:
   https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
*/

//#define USE_M5StickC
//#define USE_M5Atom
#include <M5Stack.h>
#include <Wire.h>
#include "NimBLEDevice.h"
#include "NimBLEBeacon.h"
#include "NimBLEAdvertising.h"

#include "NimBLEServer.h"
#include "debugmacros.h"
#include "esp_sleep.h"
#include "esp_system.h"

#include "BLE_TPMS.h"

#define T_PERIOD 3  // Transmission period
#define S_PERIOD 10  // Silent period
uint32_t seq = 0;

const int wdtTimeout = 15000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;
uint32_t p_millis = 0;

NimBLEScan* pBLEScan;
BLEtpms tpms[4];

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice *advertisedDevice) {
//    Serial.printf("Advertised Device: %s \n RSSI: %d \n", advertisedDevice->toString().c_str(), advertisedDevice->getRSSI());
    int tire = -1;
    if (advertisedDevice->haveManufacturerData() == true) {
      std::string data = advertisedDevice->getManufacturerData();
      if (BLEtpms::isManufacturerId(data)) {
        tire = BLEtpms::tire_id(data);
        if (tire >= 0) {
          tpms[tire].scan(data);
          tpms[tire].updated(true);
          M5.Lcd.printf("T%02d, p:%.1f, t:%.1f, b:%.0f\n", tire,
                        tpms[tire].pressure(), tpms[tire].temp() / 100.0,
                        tpms[tire].battery());
        }
      }
    }
  }
};

void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

int setAdvData(BLEAdvertising *pAdvertising, BLEtpms *tp) {
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();

  oAdvertisementData.setFlags(
      0x06);  // BR_EDR_NOT_SUPPORTED | LE General Discoverable Mode

  std::string strServiceData = "";
  strServiceData += (char)0x13;  // 長さ
  strServiceData += (char)0xff;  // AD Type 0xFF: Manufacturer specific data
  strServiceData += (char)0x00;  // Test manufacture ID low byte    
  strServiceData += (char)0x01;  // Test manufacture ID high byte

  strServiceData += (char)(0x80 + tp->tire_id());  // address 80 81 82 83
  strServiceData += (char)0xea;  // address
  strServiceData += (char)0xca;  // address 
  strServiceData += (char)((tp->dev_id()>>16) & 0xff);  // tire address 11 21 31 41
  strServiceData += (char)((tp->dev_id()>>8) & 0xff);  // tire address ac aa a6 a8
  strServiceData += (char)((tp->dev_id()) & 0xff);  // tire address b7 09 c5 34
  strServiceData += (char)((tp->pressure_raw()) & 0xff);  // pressure
  strServiceData += (char)((tp->pressure_raw() >> 8) & 0xff);  // pressure
  strServiceData += (char)((tp->pressure_raw() >> 16) & 0xff);  // pressure
  strServiceData += (char)0x00;  // 
  strServiceData += (char)((tp->temp_raw()) & 0xff);  // temperature
  strServiceData += (char)((tp->temp_raw() >> 8) & 0xff);  // temperature
  strServiceData += (char)0x00;  // 
  strServiceData += (char)0x00;  // 
  strServiceData += (char)((tp->battery_raw()) & 0xff);  // battery ?
  strServiceData += (char)0x00;  // 

  oAdvertisementData.addData(strServiceData);
  pAdvertising->setAdvertisementData(oAdvertisementData);

  return 1;
}

void setup() {
  M5.begin();
  M5.Lcd.setTextFont(4);
  M5.Lcd.setTextSize(1);
  M5.Lcd.fillScreen(TFT_BLACK);

  Serial.begin(115200);
  M5.Lcd.printf("Start ESP32\n");

  tpms[0].tire_id(TIRE_FL,BLETPMS_Tire_FL);
  tpms[1].tire_id(TIRE_FR,BLETPMS_Tire_FR);
  tpms[2].tire_id(TIRE_RL,BLETPMS_Tire_RL);
  tpms[3].tire_id(TIRE_RR,BLETPMS_Tire_RR);

  /* Dummy data */
  tpms[0].temp_raw(-10000);
  tpms[0].pressure_raw(0);
  tpms[0].battery_raw(95);

  tpms[1].temp_raw(-10000);
  tpms[1].pressure_raw(0);
  tpms[1].battery_raw(60);

  tpms[2].temp_raw(-10000);
  tpms[2].pressure_raw(0);
  tpms[2].battery_raw(75);

  tpms[3].temp_raw(-10000);
  tpms[3].pressure_raw(0);
  tpms[3].battery_raw(90);
  // */

  p_millis = millis();

  NimBLEDevice::setScanFilterMode(CONFIG_BTDM_SCAN_DUPL_TYPE_DATA);
  NimBLEDevice::setScanDuplicateCacheSize(20);
  NimBLEDevice::init("TPMS_REPEAT");

  pBLEScan = NimBLEDevice::getScan(); //create new scan
  // Set the callback for when devices are discovered, no duplicates.
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
  pBLEScan->setActiveScan(true); // Set active scanning, this will get more data from the advertiser.
  pBLEScan->setInterval(100); // How often the scan occurs / switches channels; in milliseconds,
  pBLEScan->setWindow(99);  // How long to scan during the interval; in milliseconds.
  pBLEScan->setMaxResults(0); // do not store the scan results, use callback only.

}

void loop() {
  // If an error occurs that stops the scan, it will be restarted here.
  if(pBLEScan->isScanning() == false) {
      pBLEScan->start(0, nullptr, false);
  }

  if (millis() - S_PERIOD * 1000 > p_millis) {
    pBLEScan->stop();
    BLEServer *pServer = BLEDevice::createServer();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();

    for (int i = 0; i < 4; i++) {
      if (tpms[i].updated() && setAdvData(pAdvertising, &(tpms[i])) > 0) {
        pAdvertising->start();
        M5.Lcd.printf("TPMS %d Advertizing started...\n",i);
        delay(T_PERIOD * 1000);
        pAdvertising->stop();
      }
    }
    seq++;
    p_millis = millis();
    pBLEScan->start(0, nullptr, false);
  }
  M5.update();
  delay(1);
}
