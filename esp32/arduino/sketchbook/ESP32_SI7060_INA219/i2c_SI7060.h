#ifndef i2c_SI7060_h
#define i2c_SI7060_h

#include "Wire.h"
#include <Arduino.h> // for uint8_t data type

/** ######################################################################

Driver for the SI7060-Sensor

########################################################################  */

class SI7060 {

private:

    /** ######### Register-Map ################################################################# */

    static const uint8_t    I2C_ADDRESS_SI7060_00 =(0x30);
    static const uint8_t    I2C_ADDRESS_SI7060_01 =(0x31);
    static const uint8_t    I2C_ADDRESS_SI7060_02 =(0x32);
    static const uint8_t    I2C_ADDRESS_SI7060_03 =(0x33);

    static const uint8_t    CMD_SI7060_ID                          =(0xC0);
    static const uint8_t    CMD_SI7060_Dspsigm                     =(0xC1);
    static const uint8_t    CMD_SI7060_Dspsigl                     =(0xC2);
    static const uint8_t    CMD_SI7060_meas                        =(0xC4);
    static const uint8_t    CMD_SI7060_sw_op                       =(0xC6);
    static const uint8_t    CMD_SI7060_sw_hyst                     =(0xC7);

    uint8_t _address=I2C_ADDRESS_SI7060_00;
    
    uint8_t readValue(const uint8_t registerAdd);
    void writeValue(const uint8_t registerAdd, const uint8_t value);

public:

    SI7060(uint8_t type);

    void prepare();
    void setAlarm(uint8_t low4temp,float threshold,uint8_t sw_hyst);
    float readTemperature(void);
    void sleep();


};

#endif




