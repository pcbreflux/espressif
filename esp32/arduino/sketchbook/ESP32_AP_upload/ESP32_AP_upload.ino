/* Copyright (c) 2017 pcbreflux. All Rights Reserved.
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
 * 
 * @file  ESP32_AP_upload.ino 
 * 
 * @brief  WiFi Web Server File Upload
 *  
 */

#include <WiFi.h>

const char AP_DEMO_HTTP_200_IMAGE[] = "HTTP/1.1 200 OK\r\nPragma: public\r\nCache-Control: max-age=1\r\nExpires: Thu, 26 Dec 2016 23:59:59 GMT\r\nContent-Type: image/";

typedef enum {
    UPL_AP_STAT_MAIN = 1,             // GET /
    UPL_AP_STAT_LED_HIGH,             // GET /H
    UPL_AP_STAT_LED_LOW,              // GET /L
    UPL_AP_STAT_GET_IMAGE,            // GET /logo.bmp
    UPL_AP_STAT_GET_FAVICON,          // GET /favicon.ico
    UPL_AP_STAT_POST_UPLOAD,          // POST /upload
    UPL_AP_STAT_POST_START_BOUNDRY,   // POST /upload boundry
    UPL_AP_STAT_POST_GET_BOUNDRY,     // POST /upload boundry
    UPL_AP_STAT_POST_START_IMAGE,     // POST /upload image
    UPL_AP_STAT_POST_GET_IMAGE,       // POST /upload image
} UPL_AP_STAT_t;

const uint8_t favicon[] = {
0x00,0x00,0x01,0x00,0x02,0x00,0x0F,0x0E,0x02,0x00,0x01,0x00,0x01,0x00,0xA0,0x00,
0x00,0x00,0x26,0x00,0x00,0x00,0x10,0x10,0x02,0x00,0x01,0x00,0x01,0x00,0xB0,0x00,
0x00,0x00,0xC6,0x00,0x00,0x00,0x28,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x1C,0x00,
0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBF,0xFE,
0x00,0x00,0xBF,0xFE,0x00,0x00,0xBF,0xFE,0x00,0x00,0x87,0xC6,0x00,0x00,0xBB,0xBA,
0x00,0x00,0x9D,0x9E,0x00,0x00,0xBD,0xBE,0x00,0x00,0x9D,0x9E,0x00,0x00,0xBB,0xBA,
0x00,0x00,0x83,0xC6,0x00,0x00,0xFF,0xFE,0x00,0x00,0xFF,0xFE,0x00,0x00,0xFF,0xFE,
0x00,0x00,0xFF,0xFE,0x00,0x00,0x28,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x20,0x00,
0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00
};


const char* ssid     = "ESP32_TestAP";
const char* passphrase = "esp32example";

WiFiServer server(80);

#define MAX_IMAGE_SIZE 65535
#define MAX_BUF_SIZE 1024
//#define IMAGE_DEBUG 

int value = 0;
char boundbuf[MAX_BUF_SIZE];
int boundpos=0;
char imagetypebuf[MAX_BUF_SIZE];
int imagetypepos=0;
char imagebuf[MAX_IMAGE_SIZE];
int imagepos=0;

void setup() {
    bool ret;
    
    Serial.begin(115200);
 
    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Take Online ... ");
    Serial.println(ssid);

  /*
   * Set up an access point
   * @param ssid          Pointer to the SSID (max 63 char).
   * @param passphrase    (for WPA2 min 8 char, for open use NULL)
   * @param channel       WiFi channel number, 1 - 13.
   * @param ssid_hidden   Network cloaking (0 = broadcast SSID, 1 = hide SSID)
   */
    ret = WiFi.softAP(ssid, passphrase, 2, 0);

    Serial.println("");
    Serial.println("WiFi AP online ...");
    
    server.begin();

}

void printUploadForm(WiFiClient client) {
  Serial.println("printUploadForm");
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.println("<html>");
  client.println("<body>");
  client.println();
  client.println("<form action=\"upload\" method=\"post\" enctype=\"multipart/form-data\">");
  client.println("Select image to upload:");
  client.println("<input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\">");
  client.println("<input type=\"submit\" value=\"Upload Image\" name=\"submit\">");
  client.println("</form>");
  client.println();
  client.println("</body>");
  client.println("</html>");
  
  client.println();
  
}

void printFavicon(WiFiClient client) {
  Serial.println("printFavicon");
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.print(AP_DEMO_HTTP_200_IMAGE);
  client.print("png\r\n\r\n");
#ifdef IMAGE_DEBUG
  Serial.print(AP_DEMO_HTTP_200_PNG);
#endif
  for (int i=0;i<sizeof(favicon);i++) {
    client.write(favicon[i]);
#ifdef IMAGE_DEBUG
    Serial.write(favicon[i]);
#endif
  }
  
}
void printImage(WiFiClient client) {
  Serial.println("printImage");
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.print(AP_DEMO_HTTP_200_IMAGE);
  client.print(imagetypebuf);
  client.print("\r\n\r\n");
#ifdef IMAGE_DEBUG
  Serial.print(AP_DEMO_HTTP_200_PNG);
#endif
  for (int i=0;i<imagepos;i++) {
    client.write(imagebuf[i]);
#ifdef IMAGE_DEBUG
    Serial.write(imagebuf[i]);
#endif
  }
  
}

void printThanks(WiFiClient client) {
  Serial.println("printThanks");
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.println("<html>");
  client.println("<body>");
  client.println();
  client.println("Thank You");
  client.println("<a id=\"logo\" href=\"/\"><img src=\"logo.bmp\" alt=\"logo\" border=\"0\"></a>");
  client.println();
  client.println("</body>");
  client.println("</html>");
  // the content of the HTTP response follows the header:
  //client.print("Click <a href=\"/H\">here</a> turn the LED on pin 5 on<br>");
  //client.print("Click <a href=\"/L\">here</a> turn the LED on pin 5 off<br>");
  
  // The HTTP response ends with another blank line:
  client.println();
  
}

void loop(){
  int cnt;
  bool newconn=false;
  int stat;
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    stat=0;
    boundpos=0;
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      cnt = client.available();
      if (cnt) {             // if there's bytes to read from the client,
#ifdef IMAGE_DEBUG
          if (newconn==false) {
            Serial.println(cnt);
            newconn=true;
          }
#endif
          char c = client.read();             // read a byte, then
#ifndef IMAGE_DEBUG
        if (stat!=UPL_AP_STAT_POST_GET_IMAGE) {
#endif
          Serial.write(c);                    // print it out the serial monitor
#ifndef IMAGE_DEBUG
          }
#endif
          
          if (stat==UPL_AP_STAT_POST_GET_IMAGE) {
            if (imagepos<MAX_IMAGE_SIZE) {
              imagebuf[imagepos]=c;
              imagepos++;
            }
          }
          if (c == '\n') {                    // if the byte is a newline character
#ifdef IMAGE_DEBUG
            Serial.print("stat is equal=");
            Serial.println(stat);
#endif
            if (stat==UPL_AP_STAT_POST_START_BOUNDRY) {
              boundbuf[boundpos]='\0';
              boundpos++;
#ifdef IMAGE_DEBUG
              Serial.println("&&&&&&&&&&&&&&&&&");
              Serial.println(boundbuf);
              Serial.println("&&&&&&&&&&&&&&&&&");
#endif
              stat=UPL_AP_STAT_POST_UPLOAD;
              Serial.println("stats=UPL_AP_STAT_POST_UPLOAD");
            }
            if (stat==UPL_AP_STAT_POST_START_IMAGE && currentLine.length() == 0) {
              imagetypebuf[imagetypepos]='\0';
              imagetypepos++;
#ifdef IMAGE_DEBUG
              Serial.println("&&&&&&&&&&&&&&&&&");
              Serial.println(imagetypebuf);
              Serial.println("&&&&&&&&&&&&&&&&&");
#endif
              imagepos=0;
              stat=UPL_AP_STAT_POST_GET_IMAGE;
              Serial.println("stats=UPL_AP_STAT_POST_GET_IMAGE");
            }
              // if you got a newline, then clear currentLine:
            currentLine = "";
            newconn=false;
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
            if (stat==UPL_AP_STAT_POST_START_BOUNDRY) {
              if (boundpos<MAX_BUF_SIZE) {
                boundbuf[boundpos]=c;
                boundpos++;
              }
            }
            if (stat==UPL_AP_STAT_POST_START_IMAGE) {
              if (imagetypepos<MAX_BUF_SIZE) {
                imagetypebuf[imagetypepos]=c;
                imagetypepos++;
              }
            }
          }
  
          // Check to see if the client request was "GET / "
          if (currentLine.endsWith("GET / ")) {
             stat=UPL_AP_STAT_MAIN;
             Serial.println("stats=UPL_AP_STAT_MAIN");
         }
            if (currentLine.endsWith("GET /logo.bmp ")) {
            stat=UPL_AP_STAT_GET_IMAGE;
            Serial.println("stats=UPL_AP_STAT_GET_IMAGE");
         }
          if (currentLine.endsWith("GET /favicon.ico ")) {
            stat=UPL_AP_STAT_GET_FAVICON;
            Serial.println("stats=UPL_AP_STAT_GET_FAVICON");
         }
          if (currentLine.endsWith("POST /upload ")) {
            stat=UPL_AP_STAT_POST_UPLOAD;
            Serial.println("stats=UPL_AP_STAT_POST_UPLOAD");
          }
         if (stat==UPL_AP_STAT_POST_UPLOAD && currentLine.endsWith("Content-Type: multipart/form-data; boundary=")) {
            stat=UPL_AP_STAT_POST_START_BOUNDRY;
           Serial.println("stats=UPL_AP_STAT_POST_START_BOUNDRY");
         }
         if (stat==UPL_AP_STAT_POST_UPLOAD && currentLine.endsWith("Content-Type: image/")) {
            stat=UPL_AP_STAT_POST_START_IMAGE;
           Serial.println("stats=UPL_AP_STAT_POST_START_IMAGE");
          }
         if (stat==UPL_AP_STAT_POST_UPLOAD && boundpos>0 && currentLine.endsWith(boundbuf)) {
            Serial.println("found boundry");
          }
         if (stat==UPL_AP_STAT_POST_GET_IMAGE && boundpos>0 && currentLine.endsWith(boundbuf)) {
            Serial.println("found image boundry");
            Serial.println(imagepos);
            stat=UPL_AP_STAT_POST_UPLOAD;
            imagepos=imagepos-boundpos-3;
#ifdef IMAGE_DEBUG
            Serial.println(imagepos);
            for (int i=0;i<imagepos;i++) {
              Serial.write(imagebuf[i]);
            }
#endif
             Serial.println("stats=UPL_AP_STAT_POST_UPLOAD");
          }
      } else {
            if (stat==UPL_AP_STAT_MAIN) {
              printUploadForm(client);
              break;
            }
            if (stat==UPL_AP_STAT_POST_UPLOAD) {
              printThanks(client);
              break;
            }
            if (stat==UPL_AP_STAT_GET_IMAGE) {
              printImage(client);
              break;
            }
            if (stat==UPL_AP_STAT_GET_FAVICON) {
              printFavicon(client);
              break;
            }
            
           Serial.println("stat unknown");
            delay(1000);
            break;
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }

  delay(100);
 
}
