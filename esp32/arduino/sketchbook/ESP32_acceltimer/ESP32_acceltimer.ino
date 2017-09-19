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
#include <math.h>

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t timerTicksStart = 500000; // start 500000 * 1 us = 500 ms
volatile uint32_t timerTicksEnd = 20000; // start 20000 * 1 us = 20 ms

volatile uint32_t timerTicks = timerTicksStart; // start 500000 * 1 us = 500 ms
volatile uint32_t timerChange = 10000;    // change -10000 us = -10 ms
volatile double velo = 0;
volatile uint32_t steps = 0;
volatile uint32_t maxsteps = 200;
volatile uint32_t accelSteps = (timerTicksStart-timerTicksEnd)/timerChange;
volatile double ramp = 3.175;
volatile uint32_t timerTicks2 = timerTicksStart; // start 500000 * 1 us = 500 ms
volatile double velo2 = 0;
volatile uint32_t timerTicks3 = timerTicksStart; // start 500000 * 1 us = 500 ms
volatile uint32_t timerTicksEnd3 = 29729; // start 29729 * 1 us = 30 ms
volatile double velo3 = 0;
volatile double steprad = M_PI_2/accelSteps; // full circle -> 2*PI (rad) -> 360° , quater circle -> 90°-> PI/2 (rad)

void IRAM_ATTR onTimer(){
  
  // Critical Code here
  portENTER_CRITICAL_ISR(&timerMux);

  steps++;
  /* 
  // linear step change
  if (steps>=maxsteps-accelSteps) { // ramp down
    timerTicks += timerChange; 
  } else  if (steps<=accelSteps) { // ramp up
    timerTicks -= timerChange; 
  }
  */
  
  // linear acceleration
  if (steps>maxsteps-accelSteps) { // ramp down ... ToDo: less steps if ramp up and ramp down overlap
    timerTicks = (uint32_t)((double)timerTicks * (ramp * (maxsteps-steps) + 1)) / (ramp * (maxsteps-steps) + 1 - 2);
    timerTicks2 += timerChange; // just for comparison
    timerTicks3=timerTicksEnd3 / sin(steprad * ((maxsteps-steps)));
  } else  if (steps<accelSteps) { // ramp up
    timerTicks = (uint32_t)((double)timerTicks - (2*(double)timerTicks/(ramp*steps+1)));
    timerTicks2 -= timerChange;  // just for comparison
    timerTicks3=timerTicksEnd3 / sin(steprad * steps);
 }

  
  if (steps==maxsteps) {
    steps=0;
    timerTicks=timerTicksStart;
    timerTicks2=timerTicksStart;
    timerTicks3=timerTicksEnd3/sin(steprad);
  }
  timerAlarmWrite(timer, timerTicks, true); // // change timer ticks, autoreload true
  portEXIT_CRITICAL_ISR(&timerMux);

  
  // Serial.println(String("onTimer() ")+String(timerTicks)+String(" ")+String(millis()));
  velo = (1000000/(double)timerTicks);
  velo2 = (1000000/(double)timerTicks2);
  velo3 = (1000000/(double)timerTicks3);
  //Serial.println(String(velo2));
  //Serial.println(String(velo)+String(",")+String(velo2));
  Serial.println(String(velo)+String(",")+String(velo2)+String(",")+String(velo3));
  //Serial.println(String(steps)+String(" ")+String(maxsteps-steps)+String(" ")+String(timerTicks)+String(" ")+String(velo));
  //Serial.println(String(steps)+String(" ")+String(maxsteps-steps)+String(" ")+String(timerTicks)+String(" ")+String(velo));
  //Serial.println(String(steps)+String(" ")+String(maxsteps-steps)+String(" ")+String(timerTicks)+String(" ")+String(velo)+String(" ")+String(timerTicks3)+String(" ")+String(velo3));
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
