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
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
hw_timer_t * timer2 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux2 = portMUX_INITIALIZER_UNLOCKED;

volatile uint8_t led1stat = 0; 
volatile uint8_t led2stat = 0; 
volatile uint8_t led3stat = 0; 

void IRAM_ATTR onTimer0(){
  // Critical Code here
  portENTER_CRITICAL_ISR(&timerMux0);
  led1stat=1-led1stat;
  digitalWrite(16, led1stat);   // turn the LED on or off
  portEXIT_CRITICAL_ISR(&timerMux0);
}

void IRAM_ATTR onTimer1(){
  // Critical Code here
  portENTER_CRITICAL_ISR(&timerMux1);
  led2stat=1-led2stat;
  digitalWrite(17, led2stat);   // turn the LED on or off
  portEXIT_CRITICAL_ISR(&timerMux1);
}

void IRAM_ATTR onTimer2(){
  // Critical Code here
  portENTER_CRITICAL_ISR(&timerMux2);
  led3stat=1-led3stat;
  digitalWrite(5, led3stat);   // turn the LED on or off
  portEXIT_CRITICAL_ISR(&timerMux2);
}

void setup() {
  Serial.begin(115200);

  pinMode(17, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(17, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(16, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(5, LOW);    // turn the LED off by making the voltage LOW

  Serial.println("start timer 1");
  timer1 = timerBegin(1, 80, true);  // timer 1, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
  timerAttachInterrupt(timer1, &onTimer1, true); // edge (not level) triggered 
  timerAlarmWrite(timer1, 250000, true); // 250000 * 1 us = 250 ms, autoreload true

  Serial.println("start timer 0");
  timer0 = timerBegin(0, 80, true);  // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
  timerAttachInterrupt(timer0, &onTimer0, true); // edge (not level) triggered 
  timerAlarmWrite(timer0, 1000000, true); // 1000000 * 1 us = 1 s, autoreload true

  Serial.println("start timer 2");
  timer2 = timerBegin(2, 80, true);  // timer 2, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
  timerAttachInterrupt(timer2, &onTimer2, true); // edge (not level) triggered 
  timerAlarmWrite(timer2, 750000, true); // 750000 * 1 us = 750 ms, autoreload true

  // at least enable the timer alarms
  timerAlarmEnable(timer0); // enable
  timerAlarmEnable(timer1); // enable
  timerAlarmEnable(timer2); // enable
}

void loop() {
  // nope nothing here
  vTaskDelay(portMAX_DELAY); // wait as much as posible ...
}
