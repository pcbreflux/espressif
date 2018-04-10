//#include <SoftwareSerial.h>
#include "FPM.h"
//Enroll fingerprints with this example

int getFingerprintEnroll(int id);

// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE/YELLOW wire)
//SoftwareSerial mySerial(2, 3);

// pin #16 is IN from sensor (GREEN wire)
// pin #17 is OUT from arduino  (WHITE/YELLOW wire)
HardwareSerial Serial2(2);

FPM finger;

void setup()  
{
  Serial.begin(115200);
  Serial.println("fingertest");
  Serial2.begin(57600);
 
  if (finger.begin(&Serial2)) {
    Serial.println("Found fingerprint sensor!");
    Serial.print("Capacity: "); Serial.println(finger.capacity);
    Serial.print("Packet length: "); Serial.println(finger.packetLen);
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) yield();
  }

  uint8_t p = finger.getTemplateCount();
  if (p == FINGERPRINT_OK){
    Serial.print(finger.templateCount); Serial.println(" print(s) in module memory.");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
    Serial.println("Communication error!");
  else
    Serial.println("Unknown error!");

    p = finger.emptyDatabase();
    if (p == FINGERPRINT_OK){
      Serial.println("Database empty!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.print("Communication error!");
    }
    else if (p == FINGERPRINT_DBCLEARFAIL) {
      Serial.println("Could not clear database!");
    }
  
  p = finger.getTemplateCount();
  if (p == FINGERPRINT_OK){
    Serial.print(finger.templateCount); Serial.println(" print(s) in module memory.");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
    Serial.println("Communication error!");
  else
    Serial.println("Unknown error!");
}

void loop()                     // run over and over again
{
  Serial.println("Send any character to enroll a finger...");
  while (Serial.available() == 0) yield();
  Serial.println("Searching for a free slot to store the template...");
  int16_t id;
  if (get_free_id(&id))
    getFingerprintEnroll(id);
  else
    Serial.println("No free slot in flash library!");
  while (Serial.read() != -1);  // clear buffer just in case
}

bool get_free_id(int16_t * id){
  int p = -1;
  for (int page = 0; page < (finger.capacity / FPM_TEMPLATES_PER_PAGE) + 1; page++){
    p = finger.getFreeIndex(page, id);
    switch (p){
      case FINGERPRINT_OK:
        if (*id != FINGERPRINT_NOFREEINDEX){
          Serial.print("Free slot at ID ");
          Serial.println(*id);
          return true;
        }
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error!");
        return false;
      default:
        Serial.println("Unknown error!");
        return false;
    }
    yield();
  }

  return false;
}

int getFingerprintEnroll(int id) {
  int p = -1;
  Serial.println("Waiting for valid finger to enroll");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
    yield();
  }
  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
    yield();
  }

  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
    yield();
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  
  // OK converted!
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    return 0;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   

  return 0;
}
