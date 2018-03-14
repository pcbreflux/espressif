/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "BLEBeacon.h"
#include "BLEEddystoneTLM.h"
#include "BLEEddystoneURL.h"

BLEScan* pBLEScan;
int scanTime = 5; //In seconds
uint16_t beconUUID = 0xFEAA;
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00)>>8) + (((x)&0xFF)<<8))

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("\n\n");
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      std::string strServiceData = advertisedDevice.getServiceData();
       uint8_t cServiceData[100];
       strServiceData.copy((char *)cServiceData, strServiceData.length(), 0);

       if (advertisedDevice.getServiceDataUUID().equals(BLEUUID(beconUUID))==true) {  // found Eddystone UUID
        Serial.printf("is Eddystone: %d %s length %d\n", advertisedDevice.getServiceDataUUID().bitSize(), advertisedDevice.getServiceDataUUID().toString().c_str(),strServiceData.length());
        if (cServiceData[0]==0x10) {
           BLEEddystoneURL oBeacon = BLEEddystoneURL();
           oBeacon.setData(strServiceData);
           Serial.printf("Eddystone Frame Type (Eddystone-URL) ");
           Serial.printf(oBeacon.getDecodedURL().c_str());
        } else if (cServiceData[0]==0x20) {
           BLEEddystoneTLM oBeacon = BLEEddystoneTLM();
           oBeacon.setData(strServiceData);
           Serial.printf("Eddystone Frame Type (Unencrypted Eddystone-TLM) \n");
           Serial.printf(oBeacon.toString().c_str());
        } else {
          for (int i=0;i<strServiceData.length();i++) {
            Serial.printf("[%X]",cServiceData[i]);
          }
        }
        Serial.printf("\n");

       } else {
        if (advertisedDevice.haveManufacturerData()==true) {
          std::string strManufacturerData = advertisedDevice.getManufacturerData();
          
          uint8_t cManufacturerData[100];
          strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);
          
          if (strManufacturerData.length()==25 && cManufacturerData[0] == 0x4C  && cManufacturerData[1] == 0x00 ) {
            BLEBeacon oBeacon = BLEBeacon();
            oBeacon.setData(strManufacturerData);
            Serial.printf("iBeacon Frame\n");
            Serial.printf("ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n",oBeacon.getManufacturerId(),ENDIAN_CHANGE_U16(oBeacon.getMajor()),ENDIAN_CHANGE_U16(oBeacon.getMinor()),oBeacon.getProximityUUID().toString().c_str(),oBeacon.getSignalPower());
          } else {

            Serial.printf("strManufacturerData: %d ",strManufacturerData.length());
            for (int i=0;i<strManufacturerData.length();i++) {
              Serial.printf("[%X]",cManufacturerData[i]);
            }
            Serial.printf("\n");
          }
         } else {
          Serial.printf("no Beacon Advertised ServiceDataUUID: %d %s \n", advertisedDevice.getServiceDataUUID().bitSize(), advertisedDevice.getServiceDataUUID().toString().c_str());
         }
        }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime);
  Serial.printf("\nScan done! Devices found: %d\n",foundDevices.getCount());
  delay(5000);
}
