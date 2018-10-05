// KY-040 ...  ESP32
// CLK    ...  PIN 4
// DT     ...  PIN 2
// SW     ...  PIN 16
// +      ...  3.3V
// GND    ...  GND

#include <Arduino.h>
void IRAM_ATTR isrARise();
void IRAM_ATTR isrAFall();
void IRAM_ATTR isrBRise();
void IRAM_ATTR isrBFall();
void IRAM_ATTR isrSWAll();

long int rotValue=0, swValue=0;
uint8_t stateA=0, stateB=1;

#define ROTARY_PINA 2
#define ROTARY_PINB 4
#define ROTARY_PINSW 16

void IRAM_ATTR isrARise() {
 detachInterrupt(ROTARY_PINA);
 stateA=1;
 
 if(stateB==0) {
  rotValue++; // forward
 } else {
  rotValue--; // reverse
 }
 Serial.print("isrARise A ");
 Serial.print(stateA);
 Serial.print(" B ");
 Serial.print(stateB);
 Serial.print(" rotValue ");
 Serial.println(rotValue);
 attachInterrupt(ROTARY_PINA, isrAFall, FALLING);
}

void IRAM_ATTR isrAFall() {
 detachInterrupt(ROTARY_PINA);
 stateA=0;
 
 if(stateB==1) {
  rotValue++; // forward
 } else {
  rotValue--; // reverse
 }
 Serial.print("isrAFall A ");
 Serial.print(stateA);
 Serial.print(" B ");
 Serial.print(stateB);
 Serial.print(" rotValue ");
 Serial.println(rotValue);
 attachInterrupt(ROTARY_PINA, isrARise, RISING);  
}

void IRAM_ATTR isrBRise() {
 detachInterrupt(ROTARY_PINB);
 stateB=1;
 
 if(stateA==1) {
  rotValue++; // forward
 } else {
  rotValue--; // reverse
 }
 Serial.print("isrBRise A ");
 Serial.print(stateA);
 Serial.print(" B ");
 Serial.print(stateB);
 Serial.print(" rotValue ");
 Serial.println(rotValue);
 attachInterrupt(ROTARY_PINB, isrBFall, FALLING);
}

void IRAM_ATTR isrBFall() {
 detachInterrupt(ROTARY_PINB);
 stateB=0;
 
 if(stateA==0) {
  rotValue++; // forward
 } else {
   rotValue--; // reverse
 }
 Serial.print("isrBFall A ");
 Serial.print(stateA);
 Serial.print(" B ");
 Serial.print(stateB);
 Serial.print(" rotValue ");
 Serial.println(rotValue);
 attachInterrupt(ROTARY_PINB, isrBRise, RISING);
}

void IRAM_ATTR isrSWAll() {
 detachInterrupt(ROTARY_PINSW);

 swValue++;

 Serial.print("isrSWAll ");
 Serial.println(swValue);
 attachInterrupt(ROTARY_PINSW, isrSWAll, CHANGE);
}

void setup(){
  pinMode(ROTARY_PINA, INPUT_PULLUP);
  pinMode(ROTARY_PINB, INPUT_PULLUP);
  pinMode(ROTARY_PINSW, INPUT_PULLUP);

  attachInterrupt(ROTARY_PINA, isrARise, RISING);
  attachInterrupt(ROTARY_PINB, isrBRise, RISING);
  attachInterrupt(ROTARY_PINSW, isrSWAll, CHANGE);
  Serial.begin(115200);
}


void loop(){
    
}


