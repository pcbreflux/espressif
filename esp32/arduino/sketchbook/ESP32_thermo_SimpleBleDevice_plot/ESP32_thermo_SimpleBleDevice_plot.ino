// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Sketch shows how SimpleBLE to advertise the name of the device and change it on button press
// Usefull if you want to advertise some short message
// Button is attached between GPIO 0 and GND and modes are switched with each press

#include <time.h>
#include <sys/time.h>
#include "SimpleBLE.h"

SimpleBLE ble;
#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();
//uint8_t g_phyFuns;

#ifdef __cplusplus
}
#endif

uint8_t temp_farenheit;
float temp_celsius;

void onButton(){
    String out = "BLE32 at: ";
    out += String(millis() / 1000);
    Serial.println(out);
    ble.begin(out);
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    pinMode(0, INPUT_PULLUP);
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());
    ble.begin("ESP32 SimpleBLE");
    Serial.println("Press the button to change the device name");
}

void loop() {
    char strftime_buf[64];
    time_t now = 0;
    struct tm timeinfo;
    char buf[256];

    time(&now);

    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
 //   sprintf(buf,"scan start %02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
//    Serial.println(buf);

    static uint8_t lastPinState = 1;
    uint8_t pinState = digitalRead(0);
    if(!pinState && lastPinState){
        onButton();
    }
    lastPinState = pinState;
    temp_farenheit= temprature_sens_read();
    temp_celsius = ( temp_farenheit - 32 ) / 1.8;
//    Serial.print("Temp onBoard ");
//    Serial.print(temp_farenheit);
//    Serial.print("°F ");
    Serial.println(temp_celsius);
//    Serial.println("°C");
    delay(500);
    while(Serial.available()) Serial.write(Serial.read());
}
