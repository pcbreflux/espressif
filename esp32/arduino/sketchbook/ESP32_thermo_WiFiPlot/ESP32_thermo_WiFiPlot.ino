/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "WiFi.h"

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

void setup() {
    Serial.begin(115200);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("Setup done");
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


    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
//    Serial.println("scan done");
    if (n == 0) {
//        Serial.println("no networks found");
    } else {
//        Serial.print(n);
//        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
//            Serial.print(i + 1);
//            Serial.print(": ");
//            Serial.print(String(String(WiFi.SSID(i)).substring(0,3)+String("*******")));
//            Serial.print(" (");
//            Serial.print(WiFi.RSSI(i));
//            Serial.print(") ");
//            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?"Open":"Encryped");
            delay(10);
        }
    }
//    Serial.println("");
    
    temp_farenheit= temprature_sens_read();
    temp_celsius = ( temp_farenheit - 32 ) / 1.8;
//    Serial.print("Temp onBoard ");
//    Serial.print(temp_farenheit);
//    Serial.print("°F ");
    Serial.println(temp_celsius);
//    Serial.println("°C");

    // Wait a bit before scanning again
    delay(500);
}
