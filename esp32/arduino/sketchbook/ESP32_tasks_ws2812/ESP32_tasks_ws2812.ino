#include "WS2812.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

const int PIXEL_PIN1 = 23; // Avoid using any of the strapping pins on the ESP32
const int PIXEL_PIN2 = 22; 
const int PIXEL_PIN3 = 21; 
const int PIXEL_PIN4 = 19; 
const int PIXEL_PIN5 = 18; 
const int PIXEL_PIN6 = 5; 

const uint16_t NUM_PIXELS = 8;  // How many pixels you want to drive (could be set individualy)

WS2812 oPixel[] = { WS2812((gpio_num_t)PIXEL_PIN1,NUM_PIXELS,0), // pin, count, port [0..7]
                    WS2812((gpio_num_t)PIXEL_PIN2,NUM_PIXELS,1),
                    WS2812((gpio_num_t)PIXEL_PIN3,NUM_PIXELS,2),
                    WS2812((gpio_num_t)PIXEL_PIN4,NUM_PIXELS,3),
                    WS2812((gpio_num_t)PIXEL_PIN5,NUM_PIXELS,4),
                    WS2812((gpio_num_t)PIXEL_PIN6,NUM_PIXELS,5),
                  };


void myTask(void *pvParameters) {
  int taskno = (int)pvParameters;
  int sleeptime=0;
  int pos=0;
  double dHue;
  uint16_t hue;
  
   while (1) {
      for (uint16_t i=0;i<NUM_PIXELS;i++) {
        dHue = (double)360/NUM_PIXELS*(pos+i);
        hue = (uint16_t)dHue % 360;
          if (i==(pos%NUM_PIXELS)) {
          oPixel[taskno].setHSBPixel((uint16_t)i,hue, 255, 10);
        } else {
           oPixel[taskno].setPixel((uint16_t)i,0, 0, 0);
        }
      }
     oPixel[taskno].show();
     sleeptime = (int)(esp_random()&0x02FF);
     Serial.println(String("Task ")+String(taskno)+String(" ")+String(sleeptime));
     delay((sleeptime)+200);
     pos++;
  }
}

void setup() {
  Serial.begin(115200);
  for (int t=0;t<6;t++) {
      for (uint16_t i=0;i<NUM_PIXELS;i++) {
            oPixel[t].setPixel((uint16_t)i,0, 0, 0);
      }
      oPixel[t].show();
      xTaskCreatePinnedToCore(myTask, "myTask", 4096, (void *)t, 1, NULL, ARDUINO_RUNNING_CORE);
  }
}

void loop() {
  // nope nothing here
  vTaskDelay(portMAX_DELAY); // wait as much as posible ...
}
