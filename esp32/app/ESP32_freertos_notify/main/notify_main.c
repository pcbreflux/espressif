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
#include <stdlib.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static TaskHandle_t xTaskToNotify = NULL;

void tx_task(void *arg) {
	uint32_t txpos=0;

	printf("tx_task\n");
	vTaskDelay(100 / portTICK_RATE_MS); // delay arbitary 100ms wait for rx_task to set handle xTaskToNotify

	while (1) {
		printf("tx_task notify %d\n",txpos);
		if (xTaskToNotify!=NULL) {
			xTaskNotify(xTaskToNotify,txpos,eSetValueWithOverwrite);
		}
		vTaskDelay(10000 / portTICK_RATE_MS); // delay 10s
		txpos++;
	}

}

void rx_task(void *arg) {
	uint32_t rxpos;

	printf("rx_task\n");

	xTaskToNotify = xTaskGetCurrentTaskHandle();

	while (1) {
		printf("rx_task yield\n");
		xTaskNotifyWait(0,0,&rxpos,60000); // max wait 60s
		printf("rx_task get notify %d\n",rxpos);
	}
}

void app_main() {
	printf("start ESP32\n");

	printf("create two tasks\n");
    xTaskCreate(tx_task, "tx_task", 2048, NULL, 5, NULL);
    xTaskCreate(rx_task, "rx_task", 2048, NULL, 5, NULL);

	printf("end of main\n");
}
