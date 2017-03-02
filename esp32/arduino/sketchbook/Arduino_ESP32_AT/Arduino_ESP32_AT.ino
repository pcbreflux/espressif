#include <AltSoftSerial.h>
//#include <SoftwareSerial.h>

//#define CHANNELID "forUsername=EEVblog"
#define CHANNELID "id=UC2DjFE7Xf11URZqWBigcVOQ"  //  EEVblog by David (Dave) L. Jones 
//#define CHANNELID "id=UCvsMfEoIu_ZdBIgQVcY_AZA"
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//#define GOOGLEAPIKEY "...." // generate your own key visit https://console.developers.google.com
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define YTC_DELAY 30

const byte pinRX = 8; // Wire this to Tx Pin of ESP32 i.e. PIN 16
const byte pinTX = 9; // Wire this to Rx Pin of ESP32 i.e. PIN 15

//SoftwareSerial TCOM(pinRX, pinTX); // connect software serial to ESP32
AltSoftSerial ATCOM(pinRX,pinTX);    // connect alt software serial to ESP32
int SendSite(String server,int port, String type, String request) {
  int ret=0;
     ATCOM.println("AT+CIPSTART=\""+type+"\",\""+server+"\","+String(port));
     if (type=="SSL") {
      delay(3000);
     }else {
      delay(500);
     }
      while (ATCOM.available()){
        String inData = ATCOM.readStringUntil('\n');
        //Serial.println("AT: " + inData);
        if (inData.indexOf("ERROR")>=0) {
          Serial.println("AT: " + inData);
          ret=1;
          break;
        }
      }  
      if (ret==0) {
        ATCOM.println("AT+CIPSEND="+String(request.length()));
        delay(300);
        while (ATCOM.available()){
          String inData = ATCOM.readStringUntil('\n');
          //Serial.println("AT: " + inData);
          if (inData.indexOf("ERROR")>=0) {
            Serial.println("AT: " + inData);
            ret=1;
            break;
          }
        }  
        if (ret==0) {
          ATCOM.println(request);
        }
      }
      return ret;
}

void setup() {
  Serial.begin(115200);
  //ATCOM.begin(9600); // Change this to the baudrate used by ESP32 Run AT+UART_DEF=9600,8,1,0,0 on ESP32
  ATCOM.begin(74880); // ESP32-AT default (or Run AT+UART_DEF=74880,8,1,0,0 on ESP32)
  delay(1000); // wait for ready
}

void loop() {
  String server;
  String type;
  int port;
  String request;
  
  while (Serial.available()){
    String outData = Serial.readStringUntil('\n');
    Serial.println("Serial: " + outData);
    if (outData == "g") {
      server = "www.google.com";
      type = "TCP";
      port = 80;
      request="GET /search?q=esp32 HTTP/1.1\nHost: www.google.de\nConnection: close\n\n";
      SendSite(server,port, type, request);
     } else if (outData == "mo") {
      server = "test.mosquitto.org";
      type = "TCP";
      port = 80;
      request="GET /index.html HTTP/1.0\nHost: test.mosquitto.org\nConnection: close\n\n";
      SendSite(server,port, type, request);
     } else if (outData == "esp32") {
      server = "www.esp32.com";
      type = "SSL";
      port = 443;
      request="GET https://www.esp32.com/index.php HTTP/1.1\nHost: www.esp32.com\nConnection: close\n\n";
      SendSite(server,port, type, request);
   } else if (outData == "ex") {
      server = "www.example.com";
      type = "SSL";
      port = 443;
      request="GET / HTTP/1.1\nHost: www.example.com\nConnection: close\n\n";
      SendSite(server,port, type, request);
     /*
   } else if (outData == "h") {
      server = "www.heise.de";
      type = "TCP";
      port = 80;
      request="GET /index.html HTTP/1.1\nHost: www.heise.de\nConnection: close\n\n";
      SendSite(server,port, type, request);
    } else if (outData == "hs") {
      server = "www.heise.de";
      type = "SSL";
      port = 443;
      request="GET https://www.heise.de/index.html HTTP/1.1\nHost: www.heise.de\nConnection: close\n\n";
      SendSite(server,port, type, request);
    */
    } else if (outData == "ytc") {
      server = "www.googleapis.com";
      type = "SSL";
      port = 443;
      request="GET https://www.googleapis.com/youtube/v3/channels?part=statistics&"+String(CHANNELID)+"&key="+String(GOOGLEAPIKEY)+" HTTP/1.1\nHost: www.googleapis.com\nConnection: close\n\n";
      if (SendSite(server,port, type, request)==0) {
        String inData = "";
        while (1) {
          if (ATCOM.available()) {
            inData = ATCOM.readStringUntil('\n');
            // Serial.println("Head: " + inData);
            if (inData.startsWith("Connection: close")) {
              break;
            }
            if (inData.startsWith("CLOSED")) {
             break;
            }
            if (inData.startsWith("ERROR")) {
             break;
            }
            if (inData.startsWith("OK")) {
             break;
            }
            if (inData.startsWith("ready")) {
             break;
            }
          }
        }
        String viewData = "";
        String subData = "";
        String videoData = "";
       while (1) {
            if (ATCOM.available()) {
              inData = ATCOM.readStringUntil('\n');
              // Serial.println("json: " + inData);
              if (inData.startsWith("CLOSED")) {
               break;
              }
              if (inData.startsWith("ERROR")) {
               break;
              }
              if (inData.startsWith("OK")) {
               break;
              }
              if (inData.startsWith("ready")) {
               break;
              }
              // Yes this is crude, qad, but works for now
              if (inData.indexOf("viewCount") >= 0) {
                viewData = inData.substring(inData.indexOf(":")+3,inData.length()-2);
              }
              if (inData.indexOf("subscriberCount") >= 0) {
                subData = inData.substring(inData.indexOf(":")+3,inData.length()-2);
              }
              if (inData.indexOf("videoCount") >= 0) {
                videoData = inData.substring(inData.indexOf(":")+3,inData.length()-1);
              }
             }
        }
        Serial.println("Subscriber: " + subData + " Views: " + viewData + " Videos: " + videoData);
      }
  } else {
      ATCOM.println(outData);
      delay(30);
   }
  }  
  /*
  while (ATCOM.available() > 0) {
      byte b = ATCOM.read();
      Serial.print(b);
  }
  */
  while (ATCOM.available()){
    String inData = ATCOM.readStringUntil('\n');
    Serial.println("ESP32: " + inData);
  }
}
