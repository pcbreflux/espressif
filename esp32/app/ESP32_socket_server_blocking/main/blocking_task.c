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

#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"

#include "sdkconfig.h"

#include "blocking_task.h"

const uint32_t gpio_pin[] = { 5,13,18,19,21,22,23,25,26,27 };

#define TAG "BLOCK"

void init_gpio(uint32_t pin) {
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (1<<pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

    gpio_set_level(pin,0);

}

void blocking_task(void *pvParameters) {
	int taskno = (int)pvParameters;
	uint32_t loop=0;
	uint32_t pin=gpio_pin[taskno-1];

	ESP_LOGI(TAG, "start blocking task %d (pin %d)",taskno, pin);
    init_gpio(pin);

	while (1) {  // for ever, no delay, no wait -> blocking
		for (loop=0;loop<10000000;loop++) {
			gpio_set_level(pin, 1);
		}
		for (loop=0;loop<10000000;loop++) {
			gpio_set_level(pin, 0);
		}
		ESP_LOGI(TAG, "repeat block loop %d",taskno);
	}

	ESP_LOGI(TAG, "All done! never reached!");

	vTaskDelete(NULL);
}


