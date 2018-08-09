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
#include "i2c_SI7060.h"

SI7060::SI7060(uint8_t type) {
    Wire.begin(); 		// I2C as Master
    if      (type == 1) _address = I2C_ADDRESS_SI7060_01;
    else if (type == 2) _address = I2C_ADDRESS_SI7060_02;
    else if (type == 3) _address = I2C_ADDRESS_SI7060_03;
    else                _address = I2C_ADDRESS_SI7060_00;
};

uint8_t SI7060::readValue(const uint8_t registerAdd) {
    uint8_t _ret;

    Wire.beginTransmission(_address);
    Wire.write(registerAdd);
    Wire.endTransmission();                   // difference to normal read
    Wire.beginTransmission(_address);   // difference to normal read
    Wire.requestFrom(_address, (uint8_t) 1);
    _ret = Wire.read();
    Wire.endTransmission();

    return _ret;
};

void SI7060::writeValue(const uint8_t registerAdd, const uint8_t value) {
  Wire.beginTransmission(_address);
  Wire.write(registerAdd);
  Wire.write(value);
  Wire.endTransmission(true);
};

void SI7060::prepare() {
  uint8_t _ret;
  _ret = readValue(CMD_SI7060_ID);
//  Serial.print("read CMD_SI7060_ID ");
//  Serial.println(_ret);

  _ret = readValue(CMD_SI7060_meas);
//  Serial.print("read CMD_SI7060_meas ");
//  Serial.println(_ret);
  
  // Prepare Mesure
  writeValue(CMD_SI7060_meas,0x04);

  _ret = readValue(CMD_SI7060_meas);
//  Serial.print("read CMD_SI7060_meas ");
//  Serial.println(_ret);
  
  writeValue(CMD_SI7060_sw_op,0x4E);
  writeValue(CMD_SI7060_sw_hyst,0x1C);

};

void SI7060::setAlarm(uint8_t low4temp,float threshold,uint8_t sw_hyst) {
  uint16_t _sw_op;
  uint8_t _sw_hyst;

  _sw_op = 256+((threshold-55)/0.4);
  if (low4temp!=0) {
    _sw_op = _sw_op || 0x200; // see _sw_op>>2
  }
  _sw_hyst = ((_sw_op&&0x03)<<6) & (sw_hyst&&0x3F);
  Serial.print("setAlarm ");
  Serial.print((uint8_t)(_sw_op>>2));
  Serial.print(" ");
  Serial.println(_sw_hyst);

  writeValue(CMD_SI7060_sw_op,(uint8_t)(_sw_op>>2));
  writeValue(CMD_SI7060_sw_hyst,_sw_hyst);
}


void SI7060::sleep() {
  // Prepare Mesure
  writeValue(CMD_SI7060_meas,0x01);
};

float SI7060::readTemperature(void) {
  float _temp;
  uint8_t _Dspsigm;
  uint8_t _Dspsigl;
  uint8_t _ret;

  _ret = readValue(CMD_SI7060_Dspsigm);
  _Dspsigm = (_ret&0x7F);
//  Serial.print("read CMD_SI7060_Dspsigm ");
//  Serial.println(_ret);
  _ret = readValue(CMD_SI7060_Dspsigl);
  _Dspsigl = _ret;
//  Serial.print("read CMD_SI7060_Dspsigl ");
//  Serial.println(_ret);

  _temp = 55+ ((float)(256*_Dspsigm)+(float)(_Dspsigl-16384))/160;
  
  return _temp;
}





