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

#include "sdkconfig.h"

#include "gpio_task.h"

#define GPIO_INPUT     0
#define GPIO_OUTPUT    13

#define TAG "OLED"

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

    //interrupt of falling edge
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (1<<GPIO_OUTPUT);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 1;
    gpio_config(&io_conf);

    gpio_set_level(GPIO_OUTPUT,0);

    //install gpio isr service
    gpio_install_isr_service(0); // no flags
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT, gpio_isr_handler, (void*) GPIO_INPUT);

}

void gpio_task(void *pvParameters) {
	EventBits_t bits;

	uint32_t loop=0;

	alarm_eventgroup = xEventGroupCreate();
    init_gpio();

	while (1) {
		bits=xEventGroupWaitBits(alarm_eventgroup, GPIO_SENSE_BIT,pdTRUE, pdFALSE, 60000 / portTICK_RATE_MS); // max wait 60s
		if(bits!=0) {
			vTaskDelay(2000 / portTICK_RATE_MS); // sleep 2s
			for (loop=0;loop<5;loop++) {
				gpio_set_level(GPIO_OUTPUT, 1);
				vTaskDelay(50 / portTICK_RATE_MS); // sleep 50ms
				gpio_set_level(GPIO_OUTPUT, 0);
				vTaskDelay(450 / portTICK_RATE_MS); // sleep 450ms
			}
			vTaskDelay(2000 / portTICK_RATE_MS); // sleep 2s
			for (loop=0;loop<5;loop++) {
				gpio_set_level(GPIO_OUTPUT, 1);
				vTaskDelay(200 / portTICK_RATE_MS); // sleep 200ms
				gpio_set_level(GPIO_OUTPUT, 0);
				vTaskDelay(300 / portTICK_RATE_MS); // sleep 300ms
			}
			gpio_set_level(GPIO_OUTPUT, 1);
			vTaskDelay(5000 / portTICK_RATE_MS); // sleep 5s
			gpio_set_level(GPIO_OUTPUT, 0);
	    	xEventGroupClearBits(alarm_eventgroup, GPIO_SENSE_BIT);
		}

		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
