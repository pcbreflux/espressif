#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

void myTask(void *pvParameters) {
  int taskno = (int)pvParameters;
  int sleeptime;
  
  Serial.println(String("start Task ")+String(taskno));

  while (1) {
     sleeptime = (int)(esp_random()&0x0F);
     Serial.println(String("Task ")+String(taskno)+String(" ")+String(sleeptime));
     delay(500+sleeptime*100);
  }
}

void setup() {
  Serial.begin(115200);
  for (int i=0;i<128;i++) {
    xTaskCreatePinnedToCore(myTask, "loop", 512, (void *)i, 1, NULL, ARDUINO_RUNNING_CORE);
    delay(300);
  }
}

void loop() {
  // nope, do nothing here
  vTaskDelay(portMAX_DELAY); // wait as much as posible ...
}
