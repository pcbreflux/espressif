#include "Arduino.h"
#include "driver/touch_pad.h"
#include "ws2812.h"

const int DATA_PIN = 22; // Avoid using any of the strapping pins on the ESP32
const uint16_t NUM_PIXELS = 300;  // How many pixels you want to drive 5mx60LEDs/m=300LEDS
uint8_t MAX_COLOR_VAL = 32; // Limits brightness

rgbVal *pixels;

static rgbVal tp_color[10] = { { 0x00, 0x00, 0x00 }, { 0x00, 0xFF, 0x00 }, { 0x00, 0x00, 0xFF }, { 0xFF, 0xFF, 0x00 }, { 0xFF, 0x00, 0xFF }, 
                               { 0x00, 0xFF, 0xFF }, { 0xFF, 0x00, 0x00 }, { 0x55, 0x55, 0x00 }, { 0x55, 0x00, 0x55 }, { 0x00, 0x55, 0x55 } };

void displayOff();

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

void displayOff() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels[i] = makeRGBVal(0, 0, 0);
  }
  ws2812_setColors(NUM_PIXELS, pixels);
}

void displayRGB(rgbVal val) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels[i] = val;
  }
  ws2812_setColors(NUM_PIXELS, pixels);
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
  pixels = (rgbVal*)malloc(sizeof(rgbVal) * NUM_PIXELS);
  if(ws2812_init(DATA_PIN, LED_WS2812B)) {
    Serial.println("Init FAILURE: halting");
    while (true) {};
  }
  displayOff();
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
              displayRGB(tp_color[i]);
              tp_activated[i] = true;
              Serial.print(i);
              Serial.print(" Touched = ");
              Serial.print(touch_value);
              Serial.print(" / ");
              Serial.println(tp_level[i]);
            } else if (tp_activated[i] == true && touch_value > tp_level[i]) {
              digitalWrite(LED_PIN, LOW);   // turn the LED off
              displayOff();
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
