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
#define LED 2
#define SENSOR 4
#define DELAY 500

int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  Serial.println("Sensor start");
}

void loop() {
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(DELAY);              // wait for a second
  
  // read the value from the sensor:
  sensorValue = analogRead(SENSOR);
  Serial.print("Value: "); Serial.println(sensorValue);
 
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(DELAY);
}
