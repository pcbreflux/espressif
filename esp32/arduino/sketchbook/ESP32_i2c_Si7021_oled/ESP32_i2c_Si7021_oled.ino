#include <U8g2lib.h>
#include <Wire.h>
#include "i2c.h"

//U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

/* Common addresses definition for temperature sensor. */
//#define SI7021_ADDR          (0x80U >> 1)
static const uint8_t SI7021_ADDR=0x40U;

static const uint8_t SI7021_REG_HUM_READ=0xF5U;  // ESP32 today without I²C clock stretching
static const uint8_t SI7021_REG_TEMP_READ=0xF3U;
static const uint8_t SI7021_REG_RESET=0xFEU;

static int pos;

uint16_t SI7021_readValue(const uint8_t cmd) {
        // Serial.println("SI7021_readValue write");
        uint16_t _ret;
        Wire.beginTransmission(SI7021_ADDR);
        Wire.write(cmd);
        Wire.endTransmission();
        delay(300);  // see SI7021 Datasheet
        // Serial.println("SI7021_readValue read");
        Wire.beginTransmission(SI7021_ADDR);   
        Wire.requestFrom(SI7021_ADDR, (uint8_t) 2);
        _ret      = (Wire.read() << 8 );
        _ret      |= Wire.read();
        Wire.endTransmission();
        return _ret;
};

inline void SI7021_getHumidity(float& rh)  {
  int32_t   _rawHumi;
  _rawHumi  = SI7021_readValue(SI7021_REG_HUM_READ);
  // Serial.print("_rawHumi ");
  // Serial.println(_rawHumi);
  rh        = (_rawHumi*125.0/65536) - 6;
};


void SI7021_getTemperature(float& celsius)  {
  int32_t   _rawTemp;
  _rawTemp  = SI7021_readValue(SI7021_REG_TEMP_READ);
  // Serial.print("_rawTemp ");
  // Serial.println(_rawTemp);
  celsius   = (_rawTemp*175.72/65536) - 46.85;
};


void setup() {
    Serial.begin(115200);
    pos=0;

    u8g2.begin();
}

void loop() {
  static float humi, temp;
  String stringOut;
  char buf[512];
  char degree = 0xB0;

  if (pos%4==0) { // Heardbeat
    u8g2.drawDisc(122,28,3);
    u8g2.sendBuffer();          // transfer internal memory to the display
  }
  
  SI7021_getHumidity(humi);
  SI7021_getTemperature(temp);
  
  stringOut = "";
  stringOut += dtostrf(humi, 3, 1, buf);
  stringOut += "% ";
  stringOut += dtostrf(temp, 3, 1, buf);
  stringOut += degree;
  stringOut += "C";

  stringOut.getBytes((unsigned char *)buf, 512, 0);

  Serial.print(pos++);
  Serial.print(" ");
  Serial.println(buf);

  u8g2.clearBuffer();          // clear the internal memory
  //u8g2.setFont(u8g2_font_ncenB14_tr); // choose a suitable font
  //u8g2.setFont(u8g2_font_6x10_tf);
  //u8g2.setFont(u8g2_font_8x13_tf);
  u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.drawStr(0,20,buf);  // write to the internal memory
  u8g2.drawBox(0,26,humi,6);
  u8g2.drawFrame(0,26,100,6);
  u8g2.sendBuffer();          // transfer internal memory to the display

  delay(100);  
}

