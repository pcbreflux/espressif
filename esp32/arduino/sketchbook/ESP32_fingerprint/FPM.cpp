/*************************************************** 
  This is an improved library for the FPM10/R305/ZFM20 optical fingerprint sensor
  Based on the Adafruit R305 library https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
  
  Written by Brian Ejike <brianrho94@gmail.com> (2017)
  Distributed under the terms of the MIT license
 ****************************************************/

#include "FPM.h"

static const uint8_t param_offsets[] = {0, 1, 2, 3, 4, 6, 7};
static const uint8_t param_sizes[] = {2, 2, 2, 2, 4, 2, 2};
static const uint16_t pLengths[] = {32, 64, 128, 256};

FPM::FPM() {
  thePassword = 0;
  theAddress = 0xFFFFFFFF;
  packetLen = 32;
  capacity = 0;
  fingerID = 0xffff;
  confidence = 0xffff;
  templateCount = 0;
}

bool FPM::begin(Stream *ss, uint32_t password, uint32_t address, uint8_t pLen) {
    mySerial = ss;
    delay(1000);            // 500 ms at least according to datasheet
    
    theAddress = address;
    thePassword = password;
    
    buffer[0] = FINGERPRINT_VERIFYPASSWORD;
    buffer[1] = (thePassword >> 24) & 0xff; buffer[2] = (thePassword >> 16) & 0xff;
    buffer[3] = (thePassword >> 8) & 0xff; buffer[4] = thePassword & 0xff;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 7, buffer);
    uint16_t len = getReply();

    if ((len != 1) || (buffer[6] != FINGERPRINT_ACKPACKET) || (buffer[9] != FINGERPRINT_OK))
      return false;

    if (readParam(DB_SIZE, &capacity) != FINGERPRINT_OK)        // get the capacity
        return false;
    if (pLen <= PACKET_256){               // set the packet length as needed
      if (setParam(SET_PACKET_LEN, pLen) == FINGERPRINT_OK){
          packetLen = pLengths[pLen];
          return true;
      }
    }
    else {
      if (readParam(PACKET_LEN, &packetLen) == FINGERPRINT_OK){     // else get the present packet length
          packetLen = pLengths[packetLen];
          return true;
      }
    }
    return false;
}

uint8_t FPM::setPassword(uint32_t pwd) {
    buffer[0] = FINGERPRINT_SETPASSWORD;
    buffer[1] = (pwd >> 24) & 0xff; buffer[2] = (pwd >> 16) & 0xff;
    buffer[3] = (pwd >> 8) & 0xff; buffer[4] = pwd & 0xff;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 7, buffer);
    uint16_t len = getReply();

    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    return buffer[9];
}

uint8_t FPM::getImage(void) {
  buffer[0] = FINGERPRINT_GETIMAGE;
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
  uint16_t len = getReply();
  
  if (buffer[6] != FINGERPRINT_ACKPACKET)
   return FINGERPRINT_BADPACKET;
  return buffer[9];
}

// for ZFM60 modules
uint8_t FPM::getImageNL(void) {
  buffer[0] = FINGERPRINT_GETIMAGE_NOLIGHT;
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
  uint16_t len = getReply();
  
  if (buffer[6] != FINGERPRINT_ACKPACKET)
   return FINGERPRINT_BADPACKET;
  return buffer[9];
}

// for ZFM60 modules
uint8_t FPM::led_on(void) {
    buffer[0] = FINGERPRINT_LEDON;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
    uint16_t len = getReply();

    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    return buffer[9];
}

// for ZFM60 modules
uint8_t FPM::led_off(void) {
    buffer[0] = FINGERPRINT_LEDOFF;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
    uint16_t len = getReply();

    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    return buffer[9];
}

uint8_t FPM::image2Tz(uint8_t slot) {
  buffer[0] = FINGERPRINT_IMAGE2TZ; 
  buffer[1] = slot;
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 4, buffer);
  uint16_t len = getReply();

  if (buffer[6] != FINGERPRINT_ACKPACKET)
   return FINGERPRINT_BADPACKET;
  return buffer[9];
}


uint8_t FPM::createModel(void) {
  buffer[0] = FINGERPRINT_REGMODEL;
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
  uint16_t len = getReply();
  
  if (buffer[6] != FINGERPRINT_ACKPACKET)
   return FINGERPRINT_BADPACKET;
  return buffer[9];
}


uint8_t FPM::storeModel(uint16_t id) {
  buffer[0] = FINGERPRINT_STORE;
  buffer[1] = 0x01;
  buffer[2] = id >> 8; buffer[3] = id & 0xFF;
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 6, buffer);
  uint16_t len = getReply();
  
  if (buffer[6] != FINGERPRINT_ACKPACKET)
   return FINGERPRINT_BADPACKET;
  return buffer[9];
}
    
//read a fingerprint template from flash into Char Buffer 1
uint8_t FPM::loadModel(uint16_t id) {
    buffer[0] = FINGERPRINT_LOAD;
    buffer[1] = 0x01;
    buffer[2] = id >> 8; buffer[3] = id & 0xFF;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 6, buffer);
    uint16_t len = getReply();
    
    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    return buffer[9];
}


uint8_t FPM::setParam(uint8_t param, uint8_t value){
	buffer[0] = FINGERPRINT_SETSYSPARAM;
    buffer[1] = param; buffer[2] = value;
	writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 5, buffer);
    uint16_t len = getReply();
    
    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    if (buffer[9] == FINGERPRINT_OK && param == SET_PACKET_LEN) // update packet length since we need it
        packetLen = pLengths[value];
    return buffer[9];
}

uint8_t FPM::readParam(uint8_t param, uint16_t * value){
    uint32_t val = *value;
    bool ret = readParam(param, &val);
    *value = val;
    return ret;
}

uint8_t FPM::readParam(uint8_t param, uint32_t * value){
    buffer[0] = FINGERPRINT_READSYSPARAM;
	writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
    uint16_t len = getReply();
    
    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    
    *value = 0;
    uint8_t * loc;
    if (buffer[9] == FINGERPRINT_OK){
        loc = &buffer[10] + param_offsets[param]*param_sizes[param];
        for (int i = 0; i < param_sizes[param]; i++){
            *((uint8_t *)value + i) = *(loc + param_sizes[param] - 1 - i);
        }
    }      
    return buffer[9];
}

// NEW: download fingerprint image to pc
uint8_t FPM::downImage(void){
	buffer[0] = FINGERPRINT_IMGUPLOAD;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
    uint16_t len = getReply();
    
    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    return buffer[9];
}

bool FPM::readRaw(void * out, uint8_t outType, bool * lastPacket, uint16_t * bufLen){
    Stream * outStream;
    uint8_t * outBuf = (uint8_t *)out;
    
    if (outType == ARRAY_TYPE) {
        if (bufLen == NULL || *bufLen < packetLen)        // if a buffer was provided, ensure theres enough space
            return false;
        else
            outBuf = (uint8_t *)out;
    }
    else if (outType == STREAM_TYPE)
        outStream = (Stream *)out;
    else
        return false;
    
    uint8_t chunk[packetLen + 12];
    uint8_t pid;
    
    uint16_t len=0;
    if (outType == ARRAY_TYPE)
        len = getReply(chunk, NULL, packetLen + 12);
    else if (outType == STREAM_TYPE)
        len = getReply(chunk, outStream, packetLen + 12);
    
    if (len != packetLen){
        return false;
    }
    
    pid = chunk[6];
    *lastPacket = false;
    
    if (pid == FINGERPRINT_DATAPACKET || pid == FINGERPRINT_ENDDATAPACKET){
        if (outType == ARRAY_TYPE){
            memcpy(outBuf, &chunk[9], len);
            *bufLen = len;
        }
        if (pid == FINGERPRINT_ENDDATAPACKET)
            *lastPacket = true;
        return true;
    }
    return false;
}

void FPM::writeRaw(uint8_t * data, uint16_t len){
    uint16_t written = 0;
    while (len > packetLen){
        writePacket(theAddress, FINGERPRINT_DATAPACKET, packetLen, &data[written]);
        written += packetLen;
        len -= packetLen;
    }
    writePacket(theAddress, FINGERPRINT_ENDDATAPACKET, len, &data[written]);
}

//transfer a fingerprint template from Char Buffer 1 to host computer
uint8_t FPM::getModel(void) {
    buffer[0] = FINGERPRINT_UPLOAD;
    buffer[1] = 0x01;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 4, buffer);
    uint16_t len = getReply();
    
    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    return buffer[9];
}

uint8_t FPM::uploadModel(void){
    buffer[0] = FINGERPRINT_DOWNCHAR;
    buffer[1] = 0x01;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 4, buffer);
    uint16_t len = getReply();
    
    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    return buffer[9];
}
    
uint8_t FPM::deleteModel(uint16_t id, uint16_t num) {
    buffer[0] = FINGERPRINT_DELETE;
    buffer[1] = id >> 8; buffer[2] = id & 0xFF;
    buffer[3] = num >> 8; buffer[4] = num & 0xFF;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 7, buffer);
    uint16_t len = getReply();
        
    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    return buffer[9];
}

uint8_t FPM::emptyDatabase(void) {
  buffer[0] = FINGERPRINT_EMPTY;
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
  uint16_t len = getReply();
  
  if (buffer[6] != FINGERPRINT_ACKPACKET)
   return FINGERPRINT_BADPACKET;
  return buffer[9];
}

uint8_t FPM::fingerFastSearch(void) {
    fingerID = 0xFFFF;
    confidence = 0xFFFF;
    // high speed search of slot #1 starting at page 0 to 'capacity'
    buffer[0] = FINGERPRINT_HISPEEDSEARCH;
    buffer[1] = 0x01;
    buffer[2] = 0x00; buffer[3] = 0x00;
    buffer[4] = (uint8_t)(capacity >> 8); buffer[5] = (uint8_t)(capacity & 0xFF);
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 8, buffer);
    uint16_t len = getReply();

    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;

    if (buffer[9] == FINGERPRINT_OK){
        fingerID = buffer[10];
        fingerID <<= 8;
        fingerID |= buffer[11];

        confidence = buffer[12];
        confidence <<= 8;
        confidence |= buffer[13];
    }

    return buffer[9];
}

uint8_t FPM::match_pair(void){
    buffer[0] = FINGERPRINT_PAIRMATCH;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
    uint16_t len = getReply();

    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    
    if (buffer[9] == FINGERPRINT_OK){
        confidence = buffer[10]; 
        confidence <<= 8;
        confidence |= buffer[11];
    }

    return buffer[9];
}

uint8_t FPM::getTemplateCount(void) {
    templateCount = 0xFFFF;
    // get number of templates in memory
    buffer[0] = FINGERPRINT_TEMPLATECOUNT;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, buffer);
    uint16_t len = getReply();

    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    
    if (buffer[9] == FINGERPRINT_OK){
        templateCount = buffer[10];
        templateCount <<= 8;
        templateCount |= buffer[11];
    }

    return buffer[9];
}

uint8_t FPM::getFreeIndex(uint8_t page, int16_t * id){
    buffer[0] = FINGERPRINT_READTEMPLATEINDEX; 
    buffer[1] = page;
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 4, buffer);
    uint16_t len = getReply();
    
    if (buffer[6] != FINGERPRINT_ACKPACKET)
        return FINGERPRINT_BADPACKET;
    if (buffer[9] == FINGERPRINT_OK){
        for (int i = 0; i < len; i++){
            if (buffer[10+i] < 0xff){
                uint8_t index = 0;
                for (uint8_t bit = 0x01; bit != 0; bit <<= 1){
                    if ((bit & buffer[10+i]) == 0){
                        *id = (FPM_TEMPLATES_PER_PAGE * page) + (i * 8) + index;
                        return buffer[9];
                    }
                    index++;
                }
            }
        }
    }
    *id = FINGERPRINT_NOFREEINDEX;  // no free space found
    return buffer[9];
}

void FPM::writePacket(uint32_t addr, uint8_t packettype, 
				       uint16_t len, uint8_t *packet) {
#ifdef FINGERPRINT_DEBUG
    Serial.print("---> 0x");
    Serial.print((uint8_t)(FINGERPRINT_STARTCODE >> 8), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)FINGERPRINT_STARTCODE, HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(addr >> 24), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(addr >> 16), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(addr >> 8), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(addr), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)packettype, HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(len >> 8), HEX);
    Serial.print(" 0x");
    Serial.print((uint8_t)(len), HEX);
#endif
 
#if ARDUINO >= 100
    mySerial->write((uint8_t)(FINGERPRINT_STARTCODE >> 8));
    mySerial->write((uint8_t)FINGERPRINT_STARTCODE);
    mySerial->write((uint8_t)(addr >> 24));
    mySerial->write((uint8_t)(addr >> 16));
    mySerial->write((uint8_t)(addr >> 8));
    mySerial->write((uint8_t)(addr));
    mySerial->write((uint8_t)packettype);
    mySerial->write((uint8_t)(len >> 8));
    mySerial->write((uint8_t)(len));
#else
    mySerial->print((uint8_t)(FINGERPRINT_STARTCODE >> 8), BYTE);
    mySerial->print((uint8_t)FINGERPRINT_STARTCODE, BYTE);
    mySerial->print((uint8_t)(addr >> 24), BYTE);
    mySerial->print((uint8_t)(addr >> 16), BYTE);
    mySerial->print((uint8_t)(addr >> 8), BYTE);
    mySerial->print((uint8_t)(addr), BYTE);
    mySerial->print((uint8_t)packettype, BYTE);
    mySerial->print((uint8_t)(len >> 8), BYTE);
    mySerial->print((uint8_t)(len), BYTE);
#endif
  
    uint16_t sum = (len>>8) + (len&0xFF) + packettype;
    for (uint8_t i=0; i< len-2; i++) {
#if ARDUINO >= 100
        mySerial->write((uint8_t)(packet[i]));
#else
        mySerial->print((uint8_t)(packet[i]), BYTE);
#endif
#ifdef FINGERPRINT_DEBUG
        Serial.print(" 0x"); Serial.print(packet[i], HEX);
#endif
        sum += packet[i];
    }
#ifdef FINGERPRINT_DEBUG
    //Serial.print("Checksum = 0x"); Serial.println(sum);
    Serial.print(" 0x"); Serial.print((uint8_t)(sum>>8), HEX);
    Serial.print(" 0x"); Serial.println((uint8_t)(sum), HEX);
#endif
#if ARDUINO >= 100
    mySerial->write((uint8_t)(sum>>8));
    mySerial->write((uint8_t)sum);
#else
    mySerial->print((uint8_t)(sum>>8), BYTE);
    mySerial->print((uint8_t)sum, BYTE);
#endif
}

uint16_t FPM::getReply(uint8_t * replyBuf, Stream * outStream, uint16_t blen, uint16_t timeout) {
  uint8_t idx, val, *packet;
  uint8_t bytes = 0;
  uint16_t timer = 0;
  uint16_t len = 0;
  
  if (replyBuf != NULL)
      packet = replyBuf;
  else
      packet = buffer;
  
  memset(packet, 0xff, blen);
  
  idx = 0;
#ifdef FINGERPRINT_DEBUG
  Serial.print("<--- ");
#endif

while (true) {
    while (mySerial->available() == 0) {
      delay(5);
      timer++;
      if (timer >= timeout)
          return FINGERPRINT_TIMEOUT;
    }
    // something to read!
    val = mySerial->read();
    
    if (idx > 8 && outStream != NULL){
        outStream->write(val);
    }
    else {
        if (blen == 0)
            return 0;
        packet[idx] = val;
        blen--;
    }

    if ((idx == 0) && (packet[0] != (FINGERPRINT_STARTCODE >> 8))) {
        blen++;
        continue;
    }
    
#ifdef FINGERPRINT_DEBUG
    Serial.print(" 0x"); Serial.print(packet[idx], HEX);
#endif

    if (idx == 8) {
      if ((packet[0] != (FINGERPRINT_STARTCODE >> 8)) ||
          (packet[1] != (FINGERPRINT_STARTCODE & 0xFF)))
          return FINGERPRINT_BADPACKET;
      len = packet[7];
      len <<= 8;
      len |= packet[8];
      len -= 2;  // ignore checksum
  	}
    
    idx++;
    if (idx < len + 9)
        continue;
    else {
        mySerial->read();   // read off checksum if its arrived, to free up buffer space
        mySerial->read();
        return len;
    }
  }
}

