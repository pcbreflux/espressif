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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "sdkconfig.h"

#include "gpio_task.h"

#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880

#define GPIO_INPUT     0
#define GPIO_OUTPUT    13
//#define GPIO_OUTPUT_SPEED LEDC_LOW_SPEED_MODE // back too old git commit :-(
#define GPIO_OUTPUT_SPEED LEDC_HIGH_SPEED_MODE

#define TAG "BUZZER"

EventGroupHandle_t alarm_eventgroup;

const int GPIO_SENSE_BIT = BIT0;

void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    BaseType_t xHigherPriorityTaskWoken;
    if (gpio_num==GPIO_INPUT) {
    	xEventGroupSetBitsFromISR(alarm_eventgroup, GPIO_SENSE_BIT, &xHigherPriorityTaskWoken);
    }
}

void init_gpio(void) {
    gpio_config_t io_conf;

    //interrupt of falling edge
    io_conf.intr_type = GPIO_PIN_INTR_ANYEGDE;
    io_conf.pin_bit_mask = (1<<GPIO_INPUT);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

    //install gpio isr service
    gpio_install_isr_service(0); // no flags
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT, gpio_isr_handler, (void*) GPIO_INPUT);
}

void sound(int gpio_num,uint32_t freq,uint32_t duration) {

	ledc_timer_config_t timer_conf;
	timer_conf.speed_mode = GPIO_OUTPUT_SPEED;
	timer_conf.bit_num    = LEDC_TIMER_10_BIT;
	timer_conf.timer_num  = LEDC_TIMER_0;
	timer_conf.freq_hz    = freq;
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.gpio_num   = gpio_num;
	ledc_conf.speed_mode = GPIO_OUTPUT_SPEED;
	ledc_conf.channel    = LEDC_CHANNEL_0;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.timer_sel  = LEDC_TIMER_0;
	ledc_conf.duty       = 0x0; // 50%=0x3FFF, 100%=0x7FFF for 15 Bit
	                            // 50%=0x01FF, 100%=0x03FF for 10 Bit
	ledc_channel_config(&ledc_conf);

	// start
    ledc_set_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0, 0x7F); // 12% duty - play here for your speaker or buzzer
    ledc_update_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0);
	vTaskDelay(duration/portTICK_PERIOD_MS);
	// stop
    ledc_set_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0, 0);
    ledc_update_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0);

}

// based on https://wiki.mikrotik.com/wiki/Super_Mario_Theme
void play_theme() {
	sound(GPIO_OUTPUT,660,100);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,510,100);
	vTaskDelay(100/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,770,100);
	vTaskDelay(550/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,380,100);
	vTaskDelay(575/portTICK_PERIOD_MS);

	sound(GPIO_OUTPUT,510,100);
	vTaskDelay(450/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,380,100);
	vTaskDelay(400/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,320,100);
	vTaskDelay(500/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,440,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,480,80);
	vTaskDelay(330/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,450,100);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,430,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,380,100);
	vTaskDelay(200/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,80);
	vTaskDelay(200/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,760,50);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,860,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,700,80);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,760,50);
	vTaskDelay(350/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,80);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,520,80);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,580,80);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,480,80);
	vTaskDelay(500/portTICK_PERIOD_MS);
}
// based on http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March#Code
// original composed by John Williams for the film Star Wars: The Empire Strikes Back
void play_march(uint8_t longplay) {

    sound(GPIO_OUTPUT,a, 500);
    sound(GPIO_OUTPUT,a, 500);
    sound(GPIO_OUTPUT,a, 500);
    sound(GPIO_OUTPUT,f, 350);
    sound(GPIO_OUTPUT,cH, 150);
    sound(GPIO_OUTPUT,a, 500);
    sound(GPIO_OUTPUT,f, 350);
    sound(GPIO_OUTPUT,cH, 150);
    sound(GPIO_OUTPUT,a, 650);

    vTaskDelay(150/portTICK_PERIOD_MS);
    //end of first bit

    sound(GPIO_OUTPUT,eH, 500);
    sound(GPIO_OUTPUT,eH, 500);
    sound(GPIO_OUTPUT,eH, 500);
    sound(GPIO_OUTPUT,fH, 350);
    sound(GPIO_OUTPUT,cH, 150);
    sound(GPIO_OUTPUT,gS, 500);
    sound(GPIO_OUTPUT,f, 350);
    sound(GPIO_OUTPUT,cH, 150);
    sound(GPIO_OUTPUT,a, 650);

    vTaskDelay(150/portTICK_PERIOD_MS);
    //end of second bit...

    sound(GPIO_OUTPUT,aH, 500);
    sound(GPIO_OUTPUT,a, 300);
    sound(GPIO_OUTPUT,a, 150);
    sound(GPIO_OUTPUT,aH, 400);
    sound(GPIO_OUTPUT,gSH, 200);
    sound(GPIO_OUTPUT,gH, 200);
    sound(GPIO_OUTPUT,fSH, 125);
    sound(GPIO_OUTPUT,fH, 125);
    sound(GPIO_OUTPUT,fSH, 250);

    vTaskDelay(250/portTICK_PERIOD_MS);

    sound(GPIO_OUTPUT,aS, 250);
    sound(GPIO_OUTPUT,dSH, 400);
    sound(GPIO_OUTPUT,dH, 200);
    sound(GPIO_OUTPUT,cSH, 200);
    sound(GPIO_OUTPUT,cH, 125);
    sound(GPIO_OUTPUT,b, 125);
    sound(GPIO_OUTPUT,cH, 250);

    vTaskDelay(250/portTICK_PERIOD_MS);

    sound(GPIO_OUTPUT,f, 125);
    sound(GPIO_OUTPUT,gS, 500);
    sound(GPIO_OUTPUT,f, 375);
    sound(GPIO_OUTPUT,a, 125);
    sound(GPIO_OUTPUT,cH, 500);
    sound(GPIO_OUTPUT,a, 375);
    sound(GPIO_OUTPUT,cH, 125);
    sound(GPIO_OUTPUT,eH, 650);

    //end of third bit... (Though it doesn't play well)
    //let's repeat it
    if (longplay>=1) {
		sound(GPIO_OUTPUT,aH, 500);
		sound(GPIO_OUTPUT,a, 300);
		sound(GPIO_OUTPUT,a, 150);
		sound(GPIO_OUTPUT,aH, 400);
		sound(GPIO_OUTPUT,gSH, 200);
		sound(GPIO_OUTPUT,gH, 200);
		sound(GPIO_OUTPUT,fSH, 125);
		sound(GPIO_OUTPUT,fH, 125);
		sound(GPIO_OUTPUT,fSH, 250);

		vTaskDelay(250/portTICK_PERIOD_MS);

		sound(GPIO_OUTPUT,aS, 250);
		sound(GPIO_OUTPUT,dSH, 400);
		sound(GPIO_OUTPUT,dH, 200);
		sound(GPIO_OUTPUT,cSH, 200);
		sound(GPIO_OUTPUT,cH, 125);
		sound(GPIO_OUTPUT,b, 125);
		sound(GPIO_OUTPUT,cH, 250);

		vTaskDelay(250/portTICK_PERIOD_MS);

		sound(GPIO_OUTPUT,f, 250);
		sound(GPIO_OUTPUT,gS, 500);
		sound(GPIO_OUTPUT,f, 375);
		sound(GPIO_OUTPUT,cH, 125);
		sound(GPIO_OUTPUT,a, 500);
		sound(GPIO_OUTPUT,f, 375);
		sound(GPIO_OUTPUT,cH, 125);
		sound(GPIO_OUTPUT,a, 650);
		//end of the song
    }
}

void gpio_task(void *pvParameters) {
	EventBits_t bits;

	uint32_t loop=0;

	alarm_eventgroup = xEventGroupCreate();
    init_gpio();

	while (1) {
		bits=xEventGroupWaitBits(alarm_eventgroup, GPIO_SENSE_BIT,pdTRUE, pdFALSE, 60000 / portTICK_RATE_MS); // max wait 60s
		if(bits!=0) {
			if (loop%2==0) {
				play_march(0);
			} else {
				play_theme();
			}
	    	xEventGroupClearBits(alarm_eventgroup, GPIO_SENSE_BIT);
		}

		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
