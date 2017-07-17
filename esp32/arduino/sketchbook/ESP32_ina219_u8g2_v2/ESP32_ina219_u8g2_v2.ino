/**********************************************
* INA219 library example
* 10 May 2012 by johngineer
*
* 9 January 2016 Flavius Bindea: changed default values and begin()
*
* this code is public domain.
**********************************************/


#include <Wire.h>
#include <INA219.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

INA219 ina219;


void setup() {
  Serial.begin(115200);
  u8g2.begin();
  ina219.begin();
  // begin calls:
  // configure() with default values RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  // calibrate() with default values D_SHUNT=0.1, D_V_BUS_MAX=32, D_V_SHUNT_MAX=0.2, D_I_MAX_EXPECTED=2
  // in order to work directly with ADAFruit's INA219B breakout
}

static float shuntvoltage = 0;
static float busvoltage = 0;
static float current_mA = 0;
static float loadvoltage = 0;
static float power = 0;

static uint32_t pos = 0;

void loop() {
  char *textbuf;

  pos++;

  shuntvoltage = ina219.shuntVoltage()*1000;
  busvoltage = ina219.busVoltage();
  current_mA = ina219.shuntCurrent()*1000;
  loadvoltage = busvoltage + (shuntvoltage/1000);
  power = ina219.busPower()*1000;

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


