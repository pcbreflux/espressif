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
 * Based on the Code from Neil Kolban: https://github.com/nkolban/esp32-snippets/blob/master/hardware/infra_red/receiver/rmt_receiver.c
 */
#include "ESP32_IR_Remote.h"

const int RECV_PIN = 22; // pin on the ESP32
const int LED1_PIN = 5; // pin on the ESP32
const int LED2_PIN = 18; // pin on the ESP32

ESP32_IRrecv irrecv(RECV_PIN,3);
static uint8_t command=0;
static uint8_t led1_stat=0;
static uint8_t led2_stat=0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  irrecv.init();
  Serial.println("Init complete");
  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(LED1_PIN, led1_stat);   // turn the LED initialy off (0)
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED2_PIN, led2_stat);   // turn the LED initialy off (0)
}

void loop() {
  command=irrecv.readIR();
  if (command!=0) {
    Serial.println(command);
    if (command==22) {  // Button 1 on my remote
      led1_stat=1-led1_stat;
      digitalWrite(LED1_PIN, led1_stat);   // turn the LED on or off (1=HIGH is the voltage level)
    }
    if (command==25) {  // Button 2 on my remote
      led2_stat=1-led2_stat;
      digitalWrite(LED2_PIN, led2_stat);   // turn the LED on or off (1=HIGH is the voltage level)
    }

    
  }
}

