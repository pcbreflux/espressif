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
 */

#include "Arduino.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// DRV8825 GPIO Pins

#define M1_STEP 23
#define M1_DIR 22
#define M1_EN 21
#define M2_STEP 19
#define M2_DIR 18
#define M2_EN 5

/**@brief structur for stepping orders
 * 
 */
struct mot_send {
    int steps;
    uint32_t delayms;
};

// Handles for Multitasking

const int MOT1_BIT = BIT0;
const int MOT2_BIT = BIT1;

xQueueHandle mot1_queue;
xQueueHandle mot2_queue;
EventGroupHandle_t mot_eventgroup;

/**@brief Function for handling one Steppermotor without multitasking.
 *
 * @param[in] mot  motor number.
 * @param[in] steps number of steps to rotate (plus CW, minus CCW).
 * @param[in] delayus delay in microseconds per step.
 */
void mot_step(int mot,int steps,uint32_t delayus) {
  int m_step,m_dir,m_en;

  if (mot==2) {
    m_step=M2_STEP;
    m_dir=M2_DIR;
    m_en=M2_EN;
  } else {
    m_step=M1_STEP;
    m_dir=M1_DIR;
    m_en=M1_EN;
    
  }

    Serial.print("notask motor ");
    Serial.print(mot);
    digitalWrite(m_en, LOW);  
    if (steps > 0) {
      Serial.print(" CCW Steps ");
       digitalWrite(m_dir, HIGH);   
    } else {
      Serial.print(" CW  Steps ");
        digitalWrite(m_dir, LOW);   
    }
    steps = abs(steps);
    
    Serial.print(steps);
    Serial.print(" Speed ");
    Serial.println(delayus);
    for (int pos=0;pos<steps;pos++) {
      digitalWrite(m_step, HIGH); 
      delayMicroseconds(delayus);
      digitalWrite(m_step, LOW); 
      delayMicroseconds(delayus);
    }
}

/**@brief Function for handling one Steppermotor with multitasking.
 * Runs forever waiting for orders in the queue and after finishing 
 * set the eventgroup bit for the motor.
 *
 * @param[in] pvParameters motor number.
 */
void mot_task_step(void *pvParameters) {
  int m_step,m_dir,m_en;
  int mot = (int)pvParameters;
  int steps;
  uint32_t delayms;
  xQueueHandle mot_queue;
  struct mot_send oMotSendPara;

  if (mot==2) {
    mot_queue=mot2_queue;
    m_step=M2_STEP;
    m_dir=M2_DIR;
    m_en=M2_EN;
  } else {
    mot_queue=mot1_queue;
    m_step=M1_STEP;
    m_dir=M1_DIR;
    m_en=M1_EN;
    
  }

  while (1) {
    if(xQueueReceive(mot_queue,&oMotSendPara,60000/portTICK_RATE_MS)==pdTRUE) {  // max wait 60s
      steps = oMotSendPara.steps;
      delayms = oMotSendPara.delayms;
      Serial.print("task motor ");
      Serial.print(mot);
      digitalWrite(m_en, LOW);  
      if (steps > 0) {
        Serial.print(" CCW Steps ");
         digitalWrite(m_dir, HIGH);   
      } else {
        Serial.print(" CW  Steps ");
          digitalWrite(m_dir, LOW);   
      }
      steps = abs(steps);
      
      Serial.print(steps);
      Serial.print(" Speed ");
      Serial.println(delayms);
      Serial.print("task motor ");
      Serial.print(mot);
      Serial.println(" start");
      for (int pos=0;pos<steps;pos++) {
        digitalWrite(m_step, HIGH); 
        vTaskDelay(delayms/portTICK_RATE_MS); // delay x 1ms
        digitalWrite(m_step, LOW); 
        vTaskDelay(delayms/portTICK_RATE_MS); // delay x 1ms
      }
      Serial.print("task motor ");
      Serial.print(mot);
      Serial.println(" stop");
     if (mot==2) {
       Serial.println("mot_task_step xEventGroupSetBits 2");
       xEventGroupSetBits(mot_eventgroup, MOT2_BIT);
      } else {
       Serial.println("mot_task_step xEventGroupSetBits 1");
        xEventGroupSetBits(mot_eventgroup, MOT1_BIT);
      }
    }
    if (uxQueueMessagesWaiting(mot_queue)==0) { // no message? take a break
      vTaskDelay(100 / portTICK_RATE_MS); // delay 100ms
    }
  }
      Serial.println("mot_task_step ends");

  
}

/**@brief Function for handling one Steppermotor with multitasking.
 * send two motor stepping orders to the queue and wait for
 * all motor bits as an sync indicator.
 *
 * @param[in] steps number of steps for motor 1 to rotate (plus CW, minus CCW).
 * @param[in] delayus delay for motor 1 in miliseconds per step.
 * @param[in] steps number of steps for motor 2 to rotate (plus CW, minus CCW).
 * @param[in] delayus delay for motor 2 in miliseconds per step.
 */
int dubmot_step(int steps1, uint32_t delayms1,int steps2, uint32_t delayms2) {
  EventBits_t bits;
  struct mot_send oMotSendPara1,oMotSendPara2;
  oMotSendPara1.steps=steps1;
  oMotSendPara1.delayms=delayms1;
  oMotSendPara2.steps=steps2;
  oMotSendPara2.delayms=delayms2;

  // inter task information exchange
  Serial.println("dubmot_step xQueueSendToBack 1 ");

  if(xQueueSendToBack(mot1_queue,&oMotSendPara1,1000/portTICK_RATE_MS)!=pdTRUE) {
    Serial.println("dubmot_step error stop 1");
     return 1;
    }
   Serial.println("dubmot_step xQueueSendToBack 2 ");
   if(xQueueSendToBack(mot2_queue,&oMotSendPara2,1000/portTICK_RATE_MS)!=pdTRUE) {
    Serial.println("dubmot_step error stop 2");
      return 1;
    }

  // sync

  while (1) {
    bits=xEventGroupWaitBits(mot_eventgroup, MOT1_BIT|MOT2_BIT,pdTRUE, pdTRUE, 60000 / portTICK_RATE_MS); // max wait 60s
    if(bits==(MOT1_BIT|MOT2_BIT)) {  // xWaitForAllBits == pdTRUE, so we wait for MOT1_BIT and MOT2_BIT so all other is timeout
      Serial.println("dubmot_step sync 1+2");
      break;
    } else {
      if (uxQueueMessagesWaiting(mot1_queue)==0 && uxQueueMessagesWaiting(mot2_queue)==0 ) { // no message? take a break
        break;
      }
      Serial.println("dubmot_step wait sync");
    }

    }

   return 0;
}

/**@brief setup for GPIOs and Multitasking.
 *
 */
void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);

  // GPIO Setup

  pinMode(M1_DIR, OUTPUT);
  pinMode(M1_STEP, OUTPUT);
  pinMode(M1_EN, OUTPUT);
  digitalWrite(M1_DIR, LOW);   
  digitalWrite(M1_STEP, LOW); 
  digitalWrite(M1_EN, LOW);  

  pinMode(M2_DIR, OUTPUT);
  pinMode(M2_STEP, OUTPUT);
  pinMode(M2_EN, OUTPUT);
  digitalWrite(M2_DIR, LOW);   
  digitalWrite(M2_STEP, LOW); 
  digitalWrite(M2_EN, LOW);  

  // Multitasking setup
  
  mot1_queue = xQueueCreate(10, sizeof(mot_send));
  mot2_queue = xQueueCreate(10, sizeof(mot_send));
  mot_eventgroup = xEventGroupCreate();


  xTaskCreatePinnedToCore(mot_task_step, "mot_task_step1", 4096, (void *)1, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(mot_task_step, "mot_task_step2", 4096, (void *)2, 1, NULL, ARDUINO_RUNNING_CORE);


}

<<<<<<< HEAD
/**@brief loop single Motor driving and muliti Motor driving.
=======
/**@brief loop single Motor driving and muli Motor driving.
>>>>>>> 02e9f23ac1770668802ba4f0f50867c9aa35985d
 *
 */
void loop() {
  // put your main code here, to run repeatedly:

  // no multitasking only one Steppermoter per funktion (us)
  mot_step(1,1600,200);
  mot_step(2,1600,200);
  //mot_step(1,-6400,200);
  //mot_step(2,-6400,200);
 
  // multitasking with two Steppermoter running different steps and speed (ms)
  dubmot_step(6400, 2,-3200, 1);
  dubmot_step(800, 4,-6400, 1);
  dubmot_step(400, 8, 3200, 2);

  delay(1000);
}
