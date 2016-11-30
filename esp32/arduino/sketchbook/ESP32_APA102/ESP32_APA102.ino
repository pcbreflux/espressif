 /* Copyright (c) 2016 pcbreflux. All Rights Reserved.
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
#define clockPin 22     // GPIO-PIN
#define dataPin 23     // GPIO-PIN
#define MAX_COLORS 60  // Number of APA102 LEDs in String

static uint32_t looppos;

typedef struct colorRGBB  {
    uint8_t red, green, blue, brightness;
} colorRGBB;

void writeByte(uint8_t b) {
  uint8_t pos;
  for (pos=0;pos<=7;pos++) {
     digitalWrite(dataPin, b >> (7-pos) & 1);
     digitalWrite(clockPin, HIGH);
     digitalWrite(clockPin, LOW);
   }
}

void startFrame() {
  //Serial.println("startFrame");
  writeByte(0);
  writeByte(0);
  writeByte(0);
  writeByte(0);
}

void endFrame(uint16_t count) {
  //Serial.println("endFrame");
  writeByte(0xFF);
  writeByte(0xFF);
  writeByte(0xFF);
  writeByte(0xFF);
}

void writeRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness) {
  writeByte(0b11100000 | brightness);
  writeByte(blue);
  writeByte(green);
  writeByte(red);
}

void writeColor(colorRGBB color) {
  writeRGB(color.red, color.green, color.blue, color.brightness);
}

void writeColors(colorRGBB * colors, uint16_t count) {
  //Serial.println("writeColors");
  startFrame();
  for(uint16_t i = 0; i < count; i++) {
    writeColor(colors[i]);
  }
  endFrame(count);
} 

void setup() {
  Serial.begin(115200);
  Serial.println("Hello ESP32 from Arduino setup");
  digitalWrite(dataPin, LOW);
  pinMode(dataPin, OUTPUT);
  digitalWrite(clockPin, LOW);
  pinMode(clockPin, OUTPUT);
  looppos=0;
  randomSeed(0);
}

void ramdomBlink(uint32_t loops, uint16_t delayms) {
  colorRGBB RGB[MAX_COLORS];
  uint8_t bpos,bdir;
 
  bpos=0;
  bdir=0;
  for(uint32_t pos = 0; pos < loops; pos++) {
    for(uint16_t i = 0; i < MAX_COLORS; i++) {
      
     RGB[i].red=random(0,255);
     RGB[i].green=random(0,255);
     RGB[i].blue=random(0,255);
     RGB[i].brightness=bpos;
    }
   if (bdir==0) {
      bpos++;
    } else {
      bpos--;
    }
    if (bpos>=31) {
       bpos=30;
       bdir=1;
    }
    if (bpos==0) {
       bpos=1;
       bdir=0;
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}

void ramdomFade(uint32_t loops, uint16_t delayms) {
  uint8_t bpos,bdir;
  colorRGBB RGB[MAX_COLORS];

  bpos=0;
  bdir=0;
  for(uint32_t pos = 0; pos < loops; pos++) {
     RGB[0].red=random(0,255);
     RGB[0].green=random(0,255);
     RGB[0].blue=random(0,255);
    for(uint16_t i = 1; i < MAX_COLORS; i++) {
      
     RGB[i].red=RGB[0].red;
     RGB[i].green=RGB[0].green;
     RGB[i].blue=RGB[0].blue;
     RGB[i].brightness=bpos;
    }
   if (bdir==0) {
      bpos++;
    } else {
      bpos--;
    }
    if (bpos>=31) {
       bpos=30;
       bdir=1;
    }
    if (bpos==0) {
       bpos=1;
       bdir=0;
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}

void ramdomWalk(uint32_t loops, uint16_t delayms) {
  colorRGBB RGB[MAX_COLORS];

  for(uint32_t pos = 0; pos < loops; pos++) {
    for(uint16_t i = 0; i < MAX_COLORS; i++) {
     if (i==(pos%(MAX_COLORS))) {
       RGB[i].red=random(0,255);
       RGB[i].green=random(0,255);
       RGB[i].blue=random(0,255);
       RGB[i].brightness=31;
     } else {
       RGB[i].red=0;
       RGB[i].green=0;
       RGB[i].blue=0;
       RGB[i].brightness=0;
     }
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}

void ramdomBackWalk(uint32_t loops, uint16_t delayms) {
  colorRGBB RGB[MAX_COLORS];

  for(uint32_t pos = 0; pos < loops; pos++) {
    for(uint16_t i = 0; i < MAX_COLORS; i++) {
     if (i==((MAX_COLORS)-(pos%(MAX_COLORS)))) {
       RGB[i].red=random(0,255);
       RGB[i].green=random(0,255);
       RGB[i].blue=random(0,255);
       RGB[i].brightness=31;
     } else {
       RGB[i].red=0;
       RGB[i].green=0;
       RGB[i].blue=0;
       RGB[i].brightness=0;
     }
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}


void ramdomStep(uint32_t loops, uint16_t delayms) {
  colorRGBB RGB[MAX_COLORS];

  for(uint32_t pos = 0; pos < loops; pos++) {
    for(uint16_t i = 0; i < MAX_COLORS; i++) {
     if (i<=(pos%(MAX_COLORS))) {
       RGB[i].red=random(0,255);
       RGB[i].green=random(0,255);
       RGB[i].blue=random(0,255);
       RGB[i].brightness=31;
     } else {
       RGB[i].red=0;
       RGB[i].green=0;
       RGB[i].blue=0;
       RGB[i].brightness=0;
     }
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}


void ramdomStepR(uint32_t loops, uint16_t delayms) {
  colorRGBB RGB[MAX_COLORS];

  for(uint32_t pos = 0; pos < loops; pos++) {
    for(uint16_t i = 0; i < MAX_COLORS; i++) {
     if (i<=((MAX_COLORS)-(pos%(MAX_COLORS)))) {
       RGB[i].red=random(0,255);
       RGB[i].green=random(0,255);
       RGB[i].blue=random(0,255);
       RGB[i].brightness=31;
     } else {
       RGB[i].red=0;
       RGB[i].green=0;
       RGB[i].blue=0;
       RGB[i].brightness=0;
     }
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}

void ramdomBackStep(uint32_t loops, uint16_t delayms) {
  colorRGBB RGB[MAX_COLORS];

  for(uint32_t pos = 0; pos < loops; pos++) {
    for(uint16_t i = 0; i < MAX_COLORS; i++) {
     if (i>=((MAX_COLORS)-(pos%(MAX_COLORS)))) {
       RGB[i].red=random(0,255);
       RGB[i].green=random(0,255);
       RGB[i].blue=random(0,255);
       RGB[i].brightness=31;
     } else {
       RGB[i].red=0;
       RGB[i].green=0;
       RGB[i].blue=0;
       RGB[i].brightness=0;
     }
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}

void ramdomBackStepR(uint32_t loops, uint16_t delayms) {
  colorRGBB RGB[MAX_COLORS];

  for(uint32_t pos = 0; pos < loops; pos++) {
    for(uint16_t i = 0; i < MAX_COLORS; i++) {
     if (i>=(pos%(MAX_COLORS))) {
       RGB[i].red=random(0,255);
       RGB[i].green=random(0,255);
       RGB[i].blue=random(0,255);
       RGB[i].brightness=31;
     } else {
       RGB[i].red=0;
       RGB[i].green=0;
       RGB[i].blue=0;
       RGB[i].brightness=0;
     }
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}


void ramdomSingle(uint32_t loops, uint16_t delayms) {
  colorRGBB RGB[MAX_COLORS];
  uint16_t randLED;

  for(uint32_t pos = 0; pos < loops; pos++) {
    randLED=random(0,MAX_COLORS);
    for(uint16_t i = 0; i < MAX_COLORS; i++) {
     if (i==randLED) {
       RGB[i].red=random(0,255);
       RGB[i].green=random(0,255);
       RGB[i].blue=random(0,255);
       RGB[i].brightness=random(1,31);
     } else {
       RGB[i].red=0;
       RGB[i].green=0;
       RGB[i].blue=0;
       RGB[i].brightness=0;
     }
    }
     writeColors(RGB, MAX_COLORS);
     delay(delayms);

  }
}

void loop() {
  Serial.print("Hello ESP32 from Arduino loop ");
  Serial.println(looppos);

  ramdomStep(MAX_COLORS, 10);
  ramdomStepR(MAX_COLORS, 10);
  ramdomBackStep(MAX_COLORS, 10);
  ramdomBackStepR(MAX_COLORS, 10);

  ramdomWalk(MAX_COLORS,5);
  ramdomBackWalk(MAX_COLORS,5);
  ramdomSingle(200,5);
  ramdomWalk(MAX_COLORS,5);
  ramdomBackWalk(MAX_COLORS,5);

  ramdomFade(20,50);
  ramdomBlink(50,100);
  ramdomFade(20,50);
  
  ramdomWalk(MAX_COLORS,5);
  ramdomBackWalk(MAX_COLORS,5);
  ramdomWalk(MAX_COLORS,10);
  ramdomBackWalk(MAX_COLORS,10);
  ramdomWalk(MAX_COLORS,20);
  ramdomBackWalk(MAX_COLORS,20);
  ramdomWalk(MAX_COLORS,30);
  ramdomBlink(20,100);
  ramdomBackWalk(MAX_COLORS,30);
  ramdomWalk(MAX_COLORS,20);
  ramdomBackWalk(MAX_COLORS,20);
  ramdomWalk(MAX_COLORS,10);
  ramdomBackWalk(MAX_COLORS,10);
  
  looppos++;
}
