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
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
//#include "phy.h"

#include "sdkconfig.h"

#include "temp_task.h"


#define TAG "TEMP_TASK"

uint8_t temprature_sens_read();
uint8_t g_phyFuns;

void temp_task(void *pvParameters) {
	ESP_LOGI(TAG, "start temp_task");

	uint8_t temp_farenheit;
	float temp_celsius;

	while (1) {
		temp_farenheit= temprature_sens_read();
		temp_celsius = ( temp_farenheit - 32 ) / 1.8;
		ESP_LOGI(TAG, "Temp onBoard %i°F  %.2f°C", temp_farenheit, temp_celsius);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}

	ESP_LOGI(TAG, "stop temp_task");

	vTaskDelete(NULL);
}
