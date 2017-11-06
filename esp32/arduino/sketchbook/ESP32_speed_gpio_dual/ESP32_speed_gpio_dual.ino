#include <math.h>

#define PIN_CORE0 13
#define PIN_CORE1 14

#define MSG_RESULT_SZE 20

xQueueHandle result_queue;

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

  //Serial.print(String("add ")+String(count)+String(" int: "));
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
  // Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));

  return stop_millis-start_millis;
}

uint32_t add_int32(uint32_t count) {
  uint32_t pos;
  uint32_t t = 0;
  uint32_t x = 1;
  uint32_t y = 2;
  uint32_t z = 3;
  uint32_t start_millis,stop_millis;

  // Serial.print(String("add ")+String(count)+String(" int32: "));
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
  // Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));

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
  
  // Serial.print(String("div ")+String(count)+String(" int: "));
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
  //Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}
uint32_t div_int32(uint32_t count) {
  uint32_t pos;
  uint32_t t = 1<<10;
  uint32_t x = 2;
  uint32_t y = 3;
  uint32_t z = 4;
  uint32_t start_millis,stop_millis;
  
  //Serial.print(String("div ")+String(count)+String(" int32: "));
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
  //Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
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
  
  //Serial.print(String("float ")+String(count)+String(" sqrt: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    p = pos/count;
    t = sqrt(x);
    t = sqrt(y);
    t = sqrt(z);
 }
  stop_millis=millis();
  //Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
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
  
  //Serial.print(String("double ")+String(count)+String(" sqrt: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    p = pos/count;
    t = sqrt(x);
    t = sqrt(y);
    t = sqrt(z);
  }
  stop_millis=millis();
  //Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
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
  
  //Serial.print(String("float ")+String(count)+String(" sin: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    p = pos/count;
    t = sqrt(x);
    t = sqrt(y);
    t = sqrt(z);
  }
  stop_millis=millis();
  //Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
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
  
  //Serial.print(String("double ")+String(count)+String(" sin: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    p = pos/count;
    t = sqrt(x);
    t = sqrt(y);
    t = sqrt(z);
   }
  stop_millis=millis();
  //Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}

uint32_t double_pow(uint32_t count) {
  uint32_t pos;
  double t = 0;
  uint32_t start_millis,stop_millis;
  
  //Serial.print(String("double ")+String(count)+String(" pow: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) {
    t = pow(pos,2);
 }
  stop_millis=millis();
  //Serial.println(String(t)+String(" ")+String(stop_millis-start_millis)+String(" ms"));
  return stop_millis-start_millis;
}

uint32_t loop_gpio(uint32_t count,uint8_t gpio_pin) {
  uint32_t pos;
  uint32_t start_millis,stop_millis;

  //Serial.print(String("loop ")+String(count)+String(" gpio: "));
  start_millis=millis();
  for (pos=0; pos<count; pos++) { 
    digitalWrite(gpio_pin, HIGH);
    digitalWrite(gpio_pin, LOW);
  }
  stop_millis=millis();
  //Serial.println(String(stop_millis-start_millis)+String(" ms"));

  return stop_millis-start_millis;
}


void do_messure() {
  uint32_t msg[MSG_RESULT_SZE];
  uint32_t total_millis=0;
  uint8_t pos=0;

  msg[0] = xPortGetCoreID();

  if (msg[0]==0) {
    msg[1] = loop_gpio(500000,PIN_CORE0);
  } else { 
    msg[1] = loop_gpio(500000,PIN_CORE1);
  }
  total_millis+=msg[1];
//  msg[2] = add_int(1000000);
//  total_millis+=msg[2];
//  msg[3] = add_int32(1000000);
//  total_millis+=msg[3];
//  msg[4] = div_int(1000000);
//  total_millis+=msg[4];
//  msg[5] = div_int32(1000000);
//  total_millis+=msg[5];
//  msg[6] = float_sqrt(500000);
//  total_millis+=msg[6];
//  msg[7] = double_sqrt(500000);
//  total_millis+=msg[7];
//  msg[8] = float_sin(100000);
//  total_millis+=msg[8];
//  msg[9] = double_sin(100000);
//  total_millis+=msg[9];
  msg[10] = double_pow(50000);
  total_millis+=msg[10];
  
  msg[11] = total_millis;

  xQueueSendToBack(result_queue,&msg,portMAX_DELAY);
  
}

void myCore0Task(void *pvParameters) {
  int taskno = (int)pvParameters;
  
  Serial.println(String("start Task on core ")+String(xPortGetCoreID()));

  while (1) {
    do_messure();
    delay(1000);
  }
}

void myCore1Task(void *pvParameters) {
  int taskno = (int)pvParameters;
  
  Serial.println(String("start Task on core ")+String(xPortGetCoreID()));

  while (1) {
    do_messure();
    delay(1000);
  }
}

void myResultTask(void *pvParameters) {
  int taskno = (int)pvParameters;
  uint32_t msg[MSG_RESULT_SZE];
  uint8_t pos=0;
  
  Serial.println(String("start result Task on core ")+String(xPortGetCoreID()));

  while (1) {
    if(xQueueReceive(result_queue,&msg,portMAX_DELAY)==pdTRUE) {  // max wait
      Serial.println(String("core ")+String(msg[0]));
      Serial.println(String("loop 500000 gpio: ")+String(msg[1])+String(" ms"));
//      Serial.println(String("add_int ")+String(msg[2])+String(" ms"));
//      Serial.println(String("add_int32 ")+String(msg[3])+String(" ms"));
//      Serial.println(String("div_int ")+String(msg[4])+String(" ms"));
//      Serial.println(String("div_int32 ")+String(msg[5])+String(" ms"));
//      Serial.println(String("float_sqrt ")+String(msg[6])+String(" ms"));
//      Serial.println(String("double_sqrt ")+String(msg[7])+String(" ms"));
//      Serial.println(String("float_sin ")+String(msg[8])+String(" ms"));
//      Serial.println(String("double_sin ")+String(msg[9])+String(" ms"));
      Serial.println(String("double 50000 pow: ")+String(msg[10])+String(" ms"));
      Serial.println(String("ESP32 ---- total ")+String((double)msg[11]/1000)+String(" s\n"));
    }
  }
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_CORE0, OUTPUT);
    pinMode(PIN_CORE1, OUTPUT);

    result_queue = xQueueCreate(10, sizeof(uint32_t)*MSG_RESULT_SZE);

    xTaskCreatePinnedToCore(myCore0Task, "myCore0Task", 1024, (void *)0, 1, NULL, 0); // run on core 0
    xTaskCreatePinnedToCore(myCore1Task, "myCore1Task", 1024, (void *)1, 1, NULL, 1); // run on core 1
    xTaskCreatePinnedToCore(myResultTask, "myResultTask", 1024, (void *)1, 1, NULL, 1); // run on core 1
}

void loop() {
  // nope, do nothing here
  vTaskDelay(portMAX_DELAY); // wait as much as posible ...
}

