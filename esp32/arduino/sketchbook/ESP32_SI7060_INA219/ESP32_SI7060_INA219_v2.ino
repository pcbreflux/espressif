/* Copyright (c) 2018 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */
#include <U8g2lib.h>
#include <INA219.h>
#include "i2c_SI7060.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define DELAY 1000

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

static const uint8_t    I2C_ADDRESS_SI7060_00           =(0x30);
static const uint8_t    CMD_ID                          =(0xC0);
static const uint8_t    CMD_Dspsigm                     =(0xC1);
static const uint8_t    CMD_Dspsigl                     =(0xC2);
static const uint8_t    CMD_meas                        =(0xC4);
static const uint8_t    CMD_sw_op                       =(0xC6);
static const uint8_t    CMD_sw_hyst                     =(0xC7);
static const uint8_t    MOSFET_1                        = 25;
static const uint8_t    MOSFET_2                        = 23;
static const uint8_t    MOSFET_3                        = 23;
static const uint8_t    MOSFET_4                        = 23;
static const uint8_t    ALARM_1                         = 17;

uint8_t pin=0;
uint8_t mospin=0;
INA219 ina219;
SI7060 si7060_00(0);
SI7060 si7060_01(1);

static float shuntvoltage = 0;
static float busvoltage = 0;
static float current_mA = 0;
static float loadvoltage = 0;
static float power = 0;
static uint32_t pos = 0;

void setup()
{
  Wire.begin();
  u8g2.begin();
  
  pinMode(MOSFET_1, OUTPUT);
  pinMode(MOSFET_2, OUTPUT);
  pinMode(MOSFET_3, OUTPUT);
  pinMode(MOSFET_4, OUTPUT);
  digitalWrite(MOSFET_1, pin);   // turn the OUTPUT off (LOW is the voltage level)
  digitalWrite(MOSFET_2, pin);   // turn the OUTPUT off (LOW is the voltage level)
  digitalWrite(MOSFET_3, pin);   // turn the OUTPUT off (LOW is the voltage level)
  digitalWrite(MOSFET_4, pin);   // turn the OUTPUT off (LOW is the voltage level)
  
  pinMode(ALARM_1, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("\nI2C SI7060");
  ina219.begin();  // begin
  
}


void loop()
{
  uint8_t _alrm;
  float temp1,temp2;
  String tempmsg=String("");
  String voltmsg=String("");
  String ampmsg=String("");

  // Prepare Mesure
  si7060_00.prepare();
  temp1 = si7060_00.readTemperature();
  Serial.print("read temp1 ");
  Serial.print(temp1);
  Serial.println("°C  !");
// go to sleep
  si7060_00.sleep();
  tempmsg = String(temp1)+String("°C ");

  // Prepare Mesure
  si7060_01.prepare();
  temp2 = si7060_01.readTemperature();
  Serial.print("read temp2 ");
  Serial.print(temp2);
  Serial.println("°C  !");
// go to sleep
  si7060_01.sleep();
  tempmsg += String(temp2)+String("°C");

  pin=1-pin;
  
  _alrm = digitalRead(ALARM_1);
  Serial.println(pin==1?"PIN 1":"PIN 0");
  Serial.println(_alrm==1?"ALM 1":"ALM 0");

  
  digitalWrite(MOSFET_1, pin);   // turn the LED on (HIGH is the voltage level)
  char *textbuf;
  pos++;

  shuntvoltage = ina219.shuntVoltage()*1000;
  busvoltage = ina219.busVoltage();
  current_mA = ina219.shuntCurrent()*1000;
  loadvoltage = busvoltage + (shuntvoltage/1000);
//  power = ina219.busPower()*1000;
  power = busvoltage*current_mA;

  if (_alrm==1) {
    mospin = 0;
  } else {
    if (loadvoltage>4.2) {
      mospin = 1;
    } else {
      mospin = 0;
    }
  }
  digitalWrite(MOSFET_2, mospin);

  textbuf = (char *)String(String("V+ ")+String(loadvoltage)+String(" V ")).c_str();
  Serial.print(textbuf);
  voltmsg += String(textbuf);

  textbuf = (char *)String(String("V- ")+String(busvoltage)+String(" V ")).c_str();
  Serial.print(textbuf);
  voltmsg += String(textbuf);

  textbuf = (char *)String(String("dV ")+String(shuntvoltage)+String(" mV ")).c_str();
  Serial.print(textbuf);
  
  textbuf = (char *)String(String("I ")+String(current_mA)+String(" mA ")).c_str();
  Serial.print(textbuf);
  ampmsg += String(textbuf);
  
  textbuf = (char *)String(String("P ")+String(power)+String(" mW ")).c_str();
  Serial.println(textbuf);
  ampmsg += String(textbuf);

  u8g2.clearBuffer();          // clear the internal memory
  
  u8g2.setFont(u8g2_font_fur14_tf); // choose a suitable font
  u8g2.drawUTF8(0,16,tempmsg.c_str());  // write something to the internal memory
  u8g2.setFont(u8g2_font_helvR08_tf); // choose a suitable font
  u8g2.drawUTF8(0,33,voltmsg.c_str());  // write something to the internal memory
  u8g2.drawUTF8(0,44,ampmsg.c_str());  // write something to the internal memory
  u8g2.setFont(u8g2_font_fur11_tf); // choose a suitable font
  u8g2.drawStr(0,64,pin==1?"o":" ");  // write something to the internal memory
  u8g2.drawStr(20,64,mospin==1?"o":" ");  // write something to the internal memory
  u8g2.drawStr(40,64,_alrm==1?"o":" ");  // write something to the internal memory
  u8g2.sendBuffer();          // transfer internal memory to the display

  delay(DELAY);           // wait 5 seconds for next scan
}
