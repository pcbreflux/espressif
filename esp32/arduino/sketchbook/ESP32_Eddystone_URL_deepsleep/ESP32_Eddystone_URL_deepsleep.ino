/* Copyright (c) 2018 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */
 
/*
   Create a BLE server that will send periodic eddystone frames.
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create advertising data
   3. Start advertising.
   4. wait
   5. Stop advertising.
   6. deep sleep
   
*/
#include "sys/time.h"

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "esp_sleep.h"

#define GPIO_DEEP_SLEEP_DURATION     1  // sleep 4 seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();
//uint8_t g_phyFuns;

#ifdef __cplusplus
}
#endif

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
BLEAdvertising *pAdvertising;
struct timeval now;
  
void setBeacon() {
  char beacon_data[22];
  uint16_t beconUUID = 0xFEAA;
  uint16_t volt = 3300; // 3300mV = 3.3V
  uint16_t temp = (uint16_t)((float)23.00);
  uint32_t tmil = now.tv_sec*10;
  uint8_t temp_farenheit;
  float temp_celsius;
  
  temp_farenheit= temprature_sens_read();
  temp_celsius = ( temp_farenheit - 32 ) / 1.8;
  temp = (uint16_t)(temp_celsius);
  
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  
  oAdvertisementData.setFlags(0x06); // GENERAL_DISC_MODE 0x02 | BR_EDR_NOT_SUPPORTED 0x04
  oAdvertisementData.setCompleteServices(BLEUUID(beconUUID));

    beacon_data[0] = 0x10;  // Eddystone Frame Type (Eddystone-URL)
    beacon_data[1] = 0x20;  // Beacons TX power at 0m
    beacon_data[2] = 0x03;  // URL Scheme 'https://'
    beacon_data[3] = 'g';  // URL add  1
    beacon_data[4] = 'o';  // URL add  2
    beacon_data[5] = 'o';  // URL add  3
    beacon_data[6] = '.';  // URL add  4
    beacon_data[7] = 'g';  // URL add  5
    beacon_data[8] = 'l';  // URL add  6
    beacon_data[9] = '/';  // URL add  7
    beacon_data[10] = '2';  // URL add  8
    beacon_data[11] = 'y';  // URL add  9
    beacon_data[12] = 'C';  // URL add 10
    beacon_data[13] = '6';  // URL add 11
    beacon_data[14] = 'K';  // URL add 12
    beacon_data[15] = 'X';  // URL add 13
  
  oAdvertisementData.setServiceData(BLEUUID(beconUUID), std::string(beacon_data, 16));
  
  pAdvertising->setScanResponseData(oAdvertisementData);

}

void setup() {

    
  Serial.begin(115200);
  gettimeofday(&now, NULL);

  Serial.printf("start ESP32 %d\n",bootcount++);

  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec-last);

  last = now.tv_sec;
  
  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  pAdvertising = pServer->getAdvertising();
  
  setBeacon();
   // Start advertising
  pAdvertising->start();
  Serial.println("Advertizing started...");
  delay(100);
  pAdvertising->stop();
  Serial.printf("enter deep sleep\n");
  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
  Serial.printf("in deep sleep\n");
}

void loop() {
}
