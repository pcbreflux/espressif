#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

void myTask(void *pvParameters) {
  int taskno = (int)pvParameters;
  int sleeptime;

  while (1) {
     sleeptime = (int)(esp_random()&0x0F);
     Serial.println(String("Task ")+String(taskno)+String(" ")+String(sleeptime));
     delay(500+sleeptime*100);
  }
}

void setup() {
  Serial.begin(115200);
  xTaskCreatePinnedToCore(myTask, "loop1", 4096, (void *)1, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(myTask, "loop2", 4096, (void *)2, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(myTask, "loop3", 4096, (void *)3, 1, NULL, ARDUINO_RUNNING_CORE);
}

void loop() {
  // nope, do nothing here
  vTaskDelay(portMAX_DELAY); // wait as much as posible ...
}
