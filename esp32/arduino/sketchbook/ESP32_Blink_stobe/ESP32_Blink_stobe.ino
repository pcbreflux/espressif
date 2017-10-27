#define LED_PIN 13
#define MAX_STROBE_MODE 8
portMUX_TYPE gpioMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t strobe_mode=0;

#define LEDC_CHANNEL_0     0
#define LEDC_CHANNEL_1     1
#define LEDC_TIMER_10_BIT  10
#define LEDC_BASE_FREQ     200

void IRAM_ATTR onPush(void) {
  Serial.println(String("onPush "));
  // Critical Code here
  portENTER_CRITICAL_ISR(&gpioMux);
  strobe_mode++;
  if(strobe_mode>=MAX_STROBE_MODE)  {
    strobe_mode=0;
  }
  portEXIT_CRITICAL_ISR(&gpioMux);

}
// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_PIN, OUTPUT);
  pinMode(0, INPUT);
  attachInterrupt(0,&onPush,FALLING);
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_10_BIT);
  ledcAttachPin(18, LEDC_CHANNEL_0);
 
}

void loop_gpio(uint32_t count) {
  uint32_t pos;
  for (pos=0; pos<count; pos++) { 
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);
  }
}


// the loop function runs over and over again forever
void loop() {
  Serial.println(String("strobe_mode ")+String(strobe_mode));
  switch (strobe_mode) {
  case 0:
    ledcWrite(LEDC_CHANNEL_0, 1023);
    digitalWrite(LED_PIN, HIGH); 
    delay(1000);   
    digitalWrite(LED_PIN, LOW);
    delay(1000);   
    break;
  case 1:
    ledcWrite(LEDC_CHANNEL_0, 768);
    digitalWrite(LED_PIN, HIGH); 
    delay(500);   
    digitalWrite(LED_PIN, LOW);
    delay(500);   
    break;
  case 2:
    ledcWrite(LEDC_CHANNEL_0, 512);
    digitalWrite(LED_PIN, HIGH); 
    delay(100);   
    digitalWrite(LED_PIN, LOW);
    delay(100);   
    break;
  case 3:
    ledcWrite(LEDC_CHANNEL_0, 256);
    digitalWrite(LED_PIN, HIGH); 
    delay(10);   
    digitalWrite(LED_PIN, LOW);
    delay(10);   
    break;
  case 4:
    ledcWrite(LEDC_CHANNEL_0, 128);
    digitalWrite(LED_PIN, HIGH); 
    delay(1);   
    digitalWrite(LED_PIN, LOW);
    delay(1);   
    break;
  case 5:
   ledcWrite(LEDC_CHANNEL_0, 64);
    digitalWrite(LED_PIN, HIGH); 
    delay(1);   
    digitalWrite(LED_PIN, LOW);
    delay(1);   
    break;
  case 6:
   ledcWrite(LEDC_CHANNEL_0, 32);
    digitalWrite(LED_PIN, HIGH); 
    delay(1);   
    digitalWrite(LED_PIN, LOW);
    delay(1);   
    break;
  case 7:
   ledcWrite(LEDC_CHANNEL_0, 16);
    digitalWrite(LED_PIN, HIGH); 
    delay(1);   
    digitalWrite(LED_PIN, LOW);
    delay(1);   
    break;
  case 100:
    loop_gpio(100000000);
    
    break;
  }
//  delay(1000);                       // wait for a second
}
