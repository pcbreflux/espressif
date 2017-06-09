#include "Arduino.h"
#include "driver/touch_pad.h"

#define LED_PIN 16
#define TP_LEVEL 100

static uint16_t tp_level[TOUCH_PAD_MAX];
static bool tp_activated[TOUCH_PAD_MAX];

/**@brief Function read current touch values and set activation level 
 * to halve of the value.
 * While touching the value lowers.
 */
static void tp_set_thresholds(void) {
    uint16_t touch_value;
    int32_t ret;
    
    for (int i=0; i<TOUCH_PAD_MAX; i++) {
        ret=touch_pad_read((touch_pad_t)i, &touch_value);
        if (ret==0) {
          tp_level[i] = touch_value-TP_LEVEL;
        } else {
          tp_level[i] = 512;
        }
        tp_activated[i]=false;
    }
}

/**@brief Function setup touch pad pins
 */
void setup() {
  Serial.begin(115200);
  
  // Initialize touch pad peripheral
  touch_pad_init();
  tp_set_thresholds();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);   // turn the LED off
}

/**@brief Function loop to read touch pad pins
 */
void loop() {
        uint16_t touch_value=0;
        int32_t ret;
        for (int i=0; i<TOUCH_PAD_MAX; i++) {
          ret=touch_pad_read((touch_pad_t)i, &touch_value);
          if (ret==0 && i>0) { // skip touch 0
            if (tp_activated[i] == false && touch_value < tp_level[i]) {
              digitalWrite(LED_PIN, HIGH);   // turn the LED on
              tp_activated[i] = true;
              Serial.print(i);
              Serial.print(" Touched = ");
              Serial.print(touch_value);
              Serial.print(" / ");
              Serial.println(tp_level[i]);
            } else if (tp_activated[i] == true && touch_value > tp_level[i]) {
              digitalWrite(LED_PIN, LOW);   // turn the LED off
              tp_activated[i] = false;
              Serial.print(i);
              Serial.print(" Released = ");
              Serial.print(touch_value);
              Serial.print(" / ");
              Serial.println(tp_level[i]);
            }
          }
        }
        delay(100);
}
