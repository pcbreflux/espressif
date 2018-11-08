// KY-040 ...  ESP32
// CLK    ...  PIN 4
// DT     ...  PIN 2
// SW     ...  PIN 5
// +      ...  3.3V
// GND    ...  GND

#include <Arduino.h>

long int rotValue=0, swValue=0;
uint8_t state=0;


#define ROTARY_PINA 2
#define ROTARY_PINB 4
#define ROTARY_PINSW 5

portMUX_TYPE gpioMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR isrAB() {
   uint8_t s = state & 3;

  portENTER_CRITICAL_ISR(&gpioMux);
    if (digitalRead(ROTARY_PINA)) s |= 4;
    if (digitalRead(ROTARY_PINB)) s |= 8;
    switch (s) {
      case 0: case 5: case 10: case 15:
        break;
      case 1: case 7: case 8: case 14:
        rotValue++; break;
      case 2: case 4: case 11: case 13:
        rotValue--; break;
      case 3: case 12:
        rotValue += 2; break;
      default:
        rotValue -= 2; break;
    }
    state = (s >> 2);
   portEXIT_CRITICAL_ISR(&gpioMux);
 
}


void IRAM_ATTR isrSWAll() {

 portENTER_CRITICAL_ISR(&gpioMux);
 swValue++;
 portEXIT_CRITICAL_ISR(&gpioMux);

}

void setup(){
  pinMode(ROTARY_PINA, INPUT_PULLUP);
  pinMode(ROTARY_PINB, INPUT_PULLUP);
  pinMode(ROTARY_PINSW, INPUT_PULLUP);

  attachInterrupt(ROTARY_PINA, isrAB, CHANGE);
  attachInterrupt(ROTARY_PINB, isrAB, CHANGE);
  attachInterrupt(ROTARY_PINSW, isrSWAll, CHANGE);
  Serial.begin(115200);
}


void loop(){
 Serial.print("isrSWAll ");
 Serial.print(swValue);
 Serial.print(" rotValue ");
 Serial.println(rotValue);
  delay(1000);
}


