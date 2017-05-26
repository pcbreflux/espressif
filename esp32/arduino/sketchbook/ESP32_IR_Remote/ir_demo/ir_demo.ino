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

ESP32_IRrecv irrecv(RECV_PIN);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  irrecv.init();
  Serial.println("Init complete");
}

void loop() {
  irrecv.readIR();
}

