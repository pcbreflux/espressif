/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Social networks:            http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP32 chip.

  Note: This requires ESP32 support package:
    https://github.com/espressif/arduino-esp32

  Please be sure to select the right ESP32 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "cred.h" // all credentials written to extra file

#define LEDC_CHANNEL_0     0
#define LEDC_CHANNEL_1     1
#define LEDC_TIMER_13_BIT  13
#define LEDC_BASE_FREQ     5000

BLYNK_WRITE(V2)
{
  Serial.println(String("Write V2 :")+String(param.asInt()));
  
  uint32_t duty = (8191 / 1024) * param.asInt();
  ledcWrite(LEDC_CHANNEL_0, duty);
}

BLYNK_WRITE(V3)
{
  Serial.println(String("Write V3 :")+String(param.asInt()));
  
  uint32_t duty = (8191 / 1024) * param.asInt();
  ledcWrite(LEDC_CHANNEL_1, duty);
}

uint8_t temp_farenheit;
float temp_celsius;

BLYNK_READ(V1) // Widget in the app READs Virtal Pin V1 with the certain frequency
{

  temp_farenheit= temprature_sens_read();
  temp_celsius = ( temp_farenheit - 32 ) / 1.8;
  Serial.println(String("Read V1 : ")+String(temp_celsius));
  // This command writes Arduino's uptime in seconds to Virtual Pin V1
  Blynk.virtualWrite(1, temp_celsius);
}

BLYNK_READ(V4) // Widget in the app READs Virtal Pin V4 with the certain frequency
{

  temp_farenheit= temprature_sens_read();
  temp_celsius = ( temp_farenheit - 32 ) / 1.8;
  Serial.println(String("Read V4 : ")+String(temp_celsius));
  // This command writes Arduino's uptime in seconds to Virtual Pin V4
  Blynk.virtualWrite(4, temp_celsius);
}

void setup()
{
  // Debug console
  Serial.begin(115200);
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(18, LEDC_CHANNEL_0);
  ledcAttachPin(5, LEDC_CHANNEL_1);

  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();
}

