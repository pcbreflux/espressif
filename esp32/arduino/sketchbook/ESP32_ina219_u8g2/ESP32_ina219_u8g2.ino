#include <Arduino.h>
#include <Adafruit_INA219.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

Adafruit_INA219 ina219;
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup(void) 
{
  Serial.begin(115200);
   Wire.begin();
  u8g2.begin();
  
  Serial.println("Measuring voltage and current with INA219 ...");
  ina219.begin();

}

static float ref_current_mA = 0;
static float ref_loadvoltage = 0;

static float shuntvoltage = 0;
static float busvoltage = 0;
static float current_mA = 0;
static float loadvoltage = 0;
static float power = 0;

static uint32_t pos = 0;

void loop(void) 
{
  char *textbuf;

  pos++;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  power = busvoltage * current_mA;


  if (pos < 10) {
    ref_current_mA += current_mA;
    ref_loadvoltage += loadvoltage;
  } else if (pos == 10) {
    ref_current_mA /= 10;
    ref_loadvoltage /= 10;
  } else {
    current_mA -= ref_current_mA;
    loadvoltage -= ref_loadvoltage;
  }

//  shuntvoltage += 1;
//  busvoltage += 2;
//  current_mA += 3;
//  loadvoltage += 4;

  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFont(u8g2_font_8x13_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);

  textbuf = (char *)String(String("V- ")+String(busvoltage)+String(" V ")).c_str();
  u8g2.drawStr(1,10, textbuf);
  Serial.print(textbuf);

  textbuf = (char *)String(String("dV ")+String(shuntvoltage)+String(" mV ")).c_str();
  u8g2.drawStr(1,20, textbuf);
  Serial.print(textbuf);
  
  textbuf = (char *)String(String("V+ ")+String(loadvoltage)+String(" V ")).c_str();
  u8g2.drawStr(1,30, textbuf);
  Serial.print(textbuf);
  
  textbuf = (char *)String(String("I ")+String(current_mA)+String(" mA ")).c_str();
  u8g2.drawStr(1,40, textbuf);
  Serial.print(textbuf);
  
  textbuf = (char *)String(String("P ")+String(power)+String(" mW ")).c_str();
  u8g2.drawStr(1,50, textbuf);
  Serial.println(textbuf);

  u8g2.sendBuffer();

  delay(500);
}
