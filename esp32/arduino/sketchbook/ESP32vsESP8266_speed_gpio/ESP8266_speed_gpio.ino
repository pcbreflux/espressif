#include <math.h>

/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double signalFrequency = 1000;
const double samplingFrequency = 5000;
const uint8_t amplitude = 100;
double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02

uint32_t add_int(uint32_t count) {
  uint32_t pos;
  int t = 0;
  int x = 1;
  int y = 2;
  int z = 3;
  uint32_t start_millis,stop_millis;

  Serial.print(String("add ")+String(count)+String(" int: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {  // prevent agrssive loop optimization
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
  }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));

  return stop_millis-start_millis;
}

uint32_t add_int32(uint32_t count) {
  uint32_t pos;
  uint32_t t = 0;
  uint32_t x = 1;
  uint32_t y = 2;
  uint32_t z = 3;
  uint32_t start_millis,stop_millis;

  Serial.print(String("add ")+String(count)+String(" int32: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
  }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));

  return stop_millis-start_millis;
}

/*
uint32_t add_int64(uint32_t count) {
  uint64_t pos;
  uint64_t t = 0;
  uint64_t x = 1;
  uint64_t y = 2;
  uint64_t z = 3;
  uint64_t start_millis,stop_millis;

  Serial.print(String("add ")+String(count)+String(" int: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    t += x;
    t += y;
    t += z;
    t += x;
    t += y;
    t += z;
  }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));

  return stop_millis-start_millis;
}
*/

uint32_t div_int(uint32_t count) {
  uint32_t pos;
  int t = 1<<10;
  int x = 2;
  int y = 3;
  int z = 4;
  uint32_t start_millis,stop_millis;
  
  Serial.print(String("div ")+String(count)+String(" int: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    t /= x;
    t /= y;
    t /= z;
    t /= x;
    t /= y;
    t /= z;
    if (t<=1) {
      t = 1<<10;
    }
  }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}
uint32_t div_int32(uint32_t count) {
  uint32_t pos;
  uint32_t t = 1<<10;
  uint32_t x = 2;
  uint32_t y = 3;
  uint32_t z = 4;
  uint32_t start_millis,stop_millis;
  
  Serial.print(String("div ")+String(count)+String(" int32: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    t /= x;
    t /= y;
    t /= z;
    t /= x;
    t /= y;
    t /= z;
    if (t<=1) {
      t = 1<<10;
    }
  }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}

uint32_t float_sqrt(uint32_t count) {
  uint32_t pos;
  float t = 0;
  float x = 2;
  float y = 3;
  float z = 4;
  float p = 0;
  uint32_t start_millis,stop_millis;
  
  Serial.print(String("float ")+String(count)+String(" sqrt: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    p = pos/count;
    t = sqrt(x);
    t = sqrt(y);
    t = sqrt(z);
 }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}

uint32_t double_sqrt(uint32_t count) {
  uint32_t pos;
  double t = 0;
  double x = 2;
  double y = 3;
  double z = 4;
  double p = 0;
  uint32_t start_millis,stop_millis;
  
  Serial.print(String("double ")+String(count)+String(" sqrt: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    p = pos/count;
    t = sqrt(x);
    t = sqrt(y);
    t = sqrt(z);
 }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}

uint32_t float_sin(uint32_t count) {
  uint32_t pos;
  float t = 0;
  float x = 2;
  float y = 3;
  float z = M_PI_2;
  float p = 0;
  uint32_t start_millis,stop_millis;
  
  Serial.print(String("float ")+String(count)+String(" sin: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    p = pos/count;
    t = sin(x);
    t = sin(y);
    t = sin(z);
  }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}

uint32_t double_sin(uint32_t count) {
  uint32_t pos;
  double t = 0;
  float x = 2;
  float y = 3;
  float z = M_PI_2;
  float p = 0;
  uint32_t start_millis,stop_millis;
  
  Serial.print(String("double ")+String(count)+String(" sin: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    p = pos/count;
    t = sin(x);
    t = sin(y);
    t = sin(z);
  }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}

uint32_t double_pow(uint32_t count) {
  uint32_t pos;
  double t = 0;
  uint32_t start_millis,stop_millis;
  
  Serial.print(String("double ")+String(count)+String(" pow: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    t = pow(pos,2);
 }
  stop_millis=millis();
  Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}

uint32_t loop_gpio(uint32_t count) {
  uint32_t pos;
  uint32_t start_millis,stop_millis;

  Serial.print(String("loop ")+String(count)+String(" gpio: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) { 
    digitalWrite(13, HIGH);
    digitalWrite(13, LOW);
  }
  stop_millis=millis();
  Serial.println(String(stop_millis-start_millis)+String(" ms"));

  return stop_millis-start_millis;
}


void do_messure() {
  uint32_t total_millis=0;
  total_millis += loop_gpio(1000000);
//  total_millis += add_int(1000000);
//  total_millis += add_int32(1000000);
//  total_millis += add_int64(1000000);
//  total_millis += div_int(1000000);
//  total_millis += div_int32(1000000);
//  total_millis += float_sqrt(500000);
//  total_millis += double_sqrt(500000);
//  total_millis += float_sin(100000);
//  total_millis += double_sin(100000);
  total_millis += double_pow(100000);

  #if defined(__AVR__)
    #if defined(ARDUINO_AVR_MEGA2560)
      Serial.println(String("MEGA 2560 ---- total ")+String((double)total_millis/1000)+String(" s\n"));
    #else
      Serial.println(String("Nano ---- total ")+String((double)total_millis/1000)+String(" s\n"));
    #endif
  #else
    #if defined(ESP8266)
      Serial.println(String("ESP8266 ---- total ")+String((double)total_millis/1000)+String(" s\n"));
    #else
    #if defined(ESP32)
      Serial.println(String("ESP32 ---- total ")+String((double)total_millis/1000)+String(" s\n"));
    #else
      Serial.println(String("Generic ---- total ")+String((double)total_millis/1000)+String(" s\n"));
    #endif
    #endif
  #endif
  
}


void setup() {
  #if defined(__AVR__)
    #if defined(ARDUINO_AVR_MEGA2560)
      Serial.begin(115200);
    #else
      Serial.begin(9600);
    #endif
  #else
    Serial.begin(115200);
  #endif
}

void loop() {
  do_messure();
  delay(1000);
}

