#include "sys/time.h"

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_deep_sleep.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define LED_PIN 23
#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 10 seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

void mySleep(void *pvParameters) {
  //int taskno = (int)pvParameters;

  vTaskDelay(10000 / portTICK_RATE_MS); // wait till deep sleep (10s)

  printf("enter deep sleep\n");
  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
  printf("in deep sleep\n");
}

void setup() {
  struct timeval now;

  gettimeofday(&now, NULL);

  printf("start ESP32 %d\n",bootcount++);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW

  printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec-last);
  xTaskCreatePinnedToCore(mySleep, "sleep", 4096, (void *)1, 1, NULL, ARDUINO_RUNNING_CORE);

  last = now.tv_sec;
}

void loop() {
    digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay(250 / portTICK_RATE_MS);
    digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay(250 / portTICK_RATE_MS);
}
