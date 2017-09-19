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
 */
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t timerTicks = 500000; // start 500000 * 1 us = 500 ms
volatile uint32_t timerChange = -10000;    // change -10000 us = -10 ms
volatile double velo = 0;   

void IRAM_ATTR onTimer(){
  
  // Critical Code here
  portENTER_CRITICAL_ISR(&timerMux);

  timerTicks += timerChange; 
  if (timerTicks<=20000 || timerTicks>=500000) { // 20 ms ... 500 ms
    timerChange = 0 - timerChange; // switch sign / direction
  }
  timerAlarmWrite(timer, timerTicks, true); // // change timer ticks, autoreload true
  portEXIT_CRITICAL_ISR(&timerMux);

  
  // Serial.println(String("onTimer() ")+String(timerTicks)+String(" ")+String(millis()));
  velo = (1000000/(double)timerTicks);
  Serial.println(String(velo));
}

void setup() {
  Serial.begin(115200);

  Serial.println("start timer ");
  timer = timerBegin(0, 80, true);  // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
  timerAttachInterrupt(timer, &onTimer, true); // edge (not level) triggered 
  timerAlarmWrite(timer, timerTicks, true); // 1000000 * 1 us = 1 s, autoreload true
  timerAlarmEnable(timer); // enable
}

void loop() {
  // nope nothing here
  vTaskDelay(portMAX_DELAY); // wait as much as posible ...
}
