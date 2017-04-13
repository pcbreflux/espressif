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
#define GPIO_SSR_ON    0
#define GPIO_SSR_OFF   1

#define TAG "GPIO"

void init_gpio(void) {
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (1<<GPIO_OUTPUT);
    io_conf.mode = GPIO_MODE_OUTPUT;
    if (GPIO_SSR_ON==1) { // high side, pull down
        io_conf.pull_up_en = 0;
        io_conf.pull_down_en = 1;

    } else { // low side, pull up
        io_conf.pull_up_en = 1;
        io_conf.pull_down_en = 0;

    }
   gpio_config(&io_conf);

    gpio_set_level(GPIO_OUTPUT,GPIO_SSR_OFF);

}

void gpio_task(void *pvParameters) {
	EventBits_t bits;

	uint32_t loop=0;

    init_gpio();

	while (1) {
		ESP_LOGI(TAG, "sleep 500ms");
		for (loop=0;loop<5;loop++) {
			gpio_set_level(GPIO_OUTPUT, GPIO_SSR_ON);
			vTaskDelay(500 / portTICK_RATE_MS); // sleep 500ms
			gpio_set_level(GPIO_OUTPUT, GPIO_SSR_OFF);
			vTaskDelay(500 / portTICK_RATE_MS); // sleep 500ms
		}
		ESP_LOGI(TAG, "sleep 1s");
		for (loop=0;loop<5;loop++) {
			gpio_set_level(GPIO_OUTPUT, GPIO_SSR_ON);
			vTaskDelay(1000 / portTICK_RATE_MS); // sleep 1s
			gpio_set_level(GPIO_OUTPUT, GPIO_SSR_OFF);
			vTaskDelay(1000 / portTICK_RATE_MS); // sleep 1s
		}
		ESP_LOGI(TAG, "sleep 2s");
		for (loop=0;loop<5;loop++) {
			gpio_set_level(GPIO_OUTPUT, GPIO_SSR_ON);
			vTaskDelay(2000 / portTICK_RATE_MS); // sleep 2s
			gpio_set_level(GPIO_OUTPUT, GPIO_SSR_OFF);
			vTaskDelay(2000 / portTICK_RATE_MS); // sleep 2s
		}
		ESP_LOGI(TAG, "sleep 5s");
		for (loop=0;loop<5;loop++) {
			gpio_set_level(GPIO_OUTPUT, GPIO_SSR_ON);
			vTaskDelay(5000 / portTICK_RATE_MS); // sleep 5s
			gpio_set_level(GPIO_OUTPUT, GPIO_SSR_OFF);
			vTaskDelay(5000 / portTICK_RATE_MS); // sleep 5s
		}
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
