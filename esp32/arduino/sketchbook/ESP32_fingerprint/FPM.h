/*************************************************** 
  This is an improved library for the FPM10/R305/ZFM20 optical fingerprint sensor
  Based on the Adafruit R305 library https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
  
  Written by Brian Ejike <brianrho94@gmail.com> (2017)
  Distributed under the terms of the MIT license
 ****************************************************/
#ifndef FPM_H
#define FPM_H

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

// confirmation codes
#define FINGERPRINT_OK 0x00
#define FINGERPRINT_PACKETRECIEVEERR 0x01
#define FINGERPRINT_NOFINGER 0x02
#define FINGERPRINT_IMAGEFAIL 0x03
#define FINGERPRINT_IMAGEMESS 0x06
#define FINGERPRINT_FEATUREFAIL 0x07
#define FINGERPRINT_NOMATCH 0x08
#define FINGERPRINT_NOTFOUND 0x09
#define FINGERPRINT_ENROLLMISMATCH 0x0A
#define FINGERPRINT_BADLOCATION 0x0B
#define FINGERPRINT_DBREADFAIL 0x0C
#define FINGERPRINT_UPLOADFEATUREFAIL 0x0D
#define FINGERPRINT_PACKETRESPONSEFAIL 0x0E
#define FINGERPRINT_UPLOADFAIL 0x0F
#define FINGERPRINT_DELETEFAIL 0x10
#define FINGERPRINT_DBCLEARFAIL 0x11
#define FINGERPRINT_PASSFAIL 0x13
#define FINGERPRINT_INVALIDIMAGE 0x15
#define FINGERPRINT_FLASHERR 0x18
#define FINGERPRINT_INVALIDREG 0x1A
#define FINGERPRINT_ADDRCODE 0x20
#define FINGERPRINT_PASSVERIFY 0x21

// signature and packet ids
#define FINGERPRINT_STARTCODE 0xEF01

#define FINGERPRINT_COMMANDPACKET 0x1
#define FINGERPRINT_DATAPACKET 0x2
#define FINGERPRINT_ACKPACKET 0x7
#define FINGERPRINT_ENDDATAPACKET 0x8

#define FINGERPRINT_TIMEOUT 0xFF
#define FINGERPRINT_BADPACKET 0xFE

// commands
#define FINGERPRINT_GETIMAGE 0x01
#define FINGERPRINT_IMAGE2TZ 0x02
#define FINGERPRINT_REGMODEL 0x05
#define FINGERPRINT_STORE 0x06
#define FINGERPRINT_LOAD 0x07
#define FINGERPRINT_UPLOAD 0x08
#define FINGERPRINT_DOWNCHAR    0x09
#define FINGERPRINT_IMGUPLOAD 0x0A
#define FINGERPRINT_DELETE 0x0C
#define FINGERPRINT_EMPTY 0x0D
#define FINGERPRINT_SETSYSPARAM	0x0E
#define FINGERPRINT_READSYSPARAM    0x0F
#define FINGERPRINT_VERIFYPASSWORD 0x13
#define FINGERPRINT_HISPEEDSEARCH 0x04 //0x1B
#define FINGERPRINT_TEMPLATECOUNT 0x1D
#define FINGERPRINT_READTEMPLATEINDEX   0x1F
#define FINGERPRINT_PAIRMATCH   0x03
#define FINGERPRINT_SETPASSWORD     0x12

#define FINGERPRINT_LEDON       0x50
#define FINGERPRINT_LEDOFF       0x51
#define FINGERPRINT_GETIMAGE_NOLIGHT 0x52

#define FINGERPRINT_NOFREEINDEX -2


#define FPM_MAX_PKT_LEN         32
#define FPM_PKT_OVERHEAD_LEN    12
#define FPM_BUFFER_SZ           FPM_MAX_PKT_LEN + FPM_PKT_OVERHEAD_LEN

// use these constants when setting system parameters with the setParam() method
enum {
    SET_BAUD_RATE = 4,
    SET_SEC_LEVEL,
    SET_PACKET_LEN
};

// use these constants when reading system parameters with the readParam() method
enum {
    STATUS_REG,
    SYSTEM_ID,
    DB_SIZE,
    SEC_LEVEL,
    DEVICE_ADDR,
    PACKET_LEN,
    BAUD_RATE
};

// possible values for system parameters that can be set with setPsram() 

//baud rates
enum baud_vals {
    BAUD_9600 = 1,
    BAUD_19200,
    BAUD_28800,
    BAUD_38400,
    BAUD_48000,
    BAUD_57600,
    BAUD_67200,
    BAUD_76800,
    BAUD_86400,
    BAUD_96000,
    BAUD_105600,
    BAUD_115200
};

// security levels
enum sec_levels {
    FRR_1 = 1,
    FRR_2,
    FRR_3,
    FRR_4,
    FRR_5
};

// packet lengths
enum packet_length {
    PACKET_32,
    PACKET_64,
    PACKET_128,
    PACKET_256,
    PACKET_INVALID
};

// possible output containers for template/image data read from the module
#define STREAM_TYPE 0
#define ARRAY_TYPE  1

// uncomment to enable debug output
//#define FINGERPRINT_DEBUG

// default timeout is 200 * 5 ms
#define DEFAULTTIMEOUT 200
#define FPM_TEMPLATES_PER_PAGE  256

class FPM {
    public:
        FPM();
        bool begin(Stream *ss, uint32_t password=0, uint32_t address=0xffffffff, uint8_t packetLen=PACKET_INVALID);

        uint8_t getImage(void);
        uint8_t getImageNL(void);
        uint8_t image2Tz(uint8_t slot = 1);
        uint8_t createModel(void);

        uint8_t emptyDatabase(void);
        uint8_t storeModel(uint16_t id);
        uint8_t loadModel(uint16_t id);
        uint8_t setParam(uint8_t param, uint8_t value);
        uint8_t readParam(uint8_t param, uint16_t * value);
        uint8_t readParam(uint8_t param, uint32_t * value);
        uint8_t downImage(void);
        bool readRaw(void * out, uint8_t outType, bool * lastPacket, uint16_t * bufLen=NULL);
        void writeRaw(uint8_t * data, uint16_t len);
        uint8_t getModel(void);
        uint8_t uploadModel(void);
        uint8_t deleteModel(uint16_t id, uint16_t num=1);
        uint8_t fingerFastSearch(void);
        uint8_t getTemplateCount(void);
        uint8_t getFreeIndex(uint8_t page, int16_t * id);
        uint8_t match_pair(void);
        uint8_t setPassword(uint32_t pwd);

        uint8_t led_on(void);
        uint8_t led_off(void);
        uint16_t fingerID, confidence, templateCount, packetLen, capacity;

    private: 
        uint8_t buffer[FPM_BUFFER_SZ];
        uint32_t thePassword;
        uint32_t theAddress;
        Stream * mySerial;

        void writePacket(uint32_t addr, uint8_t packettype, uint16_t len, uint8_t *packet);
        uint16_t getReply(uint8_t * replyBuf=NULL, Stream * outStream = NULL, uint16_t blen = FPM_BUFFER_SZ, uint16_t timeout=DEFAULTTIMEOUT);
};

#endif
