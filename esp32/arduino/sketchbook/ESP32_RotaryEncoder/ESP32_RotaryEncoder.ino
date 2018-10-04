// KY-040 
// CLK ... PIN 4
// DT  ... PIN 2
// SW  ... not implemented
// +   ... 3.3V
// GND ... GND



long int rotValue;
uint8_t stateA=0, stateB=1;

#define ROTARY_PINA 2
#define ROTARY_PINB 4

void setup(){
  pinMode(ROTARY_PINA, INPUT_PULLUP);
  pinMode(ROTARY_PINB, INPUT_PULLUP);

  attachInterrupt(ROTARY_PINA, isrARise, RISING);
  attachInterrupt(ROTARY_PINB, isrBRise, RISING);
  Serial.begin(115200);
}


void loop(){
    
}

void isrARise(){
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

void isrAFall() {
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

void isrBRise(){
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

void isrBFall(){
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


