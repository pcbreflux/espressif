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

#include "esp_system.h"
#include "esp_heap_alloc_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/heap_regions.h"

EventGroupHandle_t demo_eventgroup;

const int TX1_BIT = BIT0;
const int TX2_BIT = BIT1;

#define COLOR_PRINT_BLACK   "30"
#define COLOR_PRINT_RED     "31"
#define COLOR_PRINT_GREEN   "32"
#define COLOR_PRINT_BROWN   "33"
#define COLOR_PRINT_BLUE    "34"
#define COLOR_PRINT_PURPLE  "35"
#define COLOR_PRINT_CYAN    "36"
#define color_printf(COLOR,format, ... ) { printf("\033[0;" COLOR "m" format "\033[0m\n", ##__VA_ARGS__); }

void tx_task1(void *arg) {
	uint32_t txpos=0;

	color_printf(COLOR_PRINT_BLUE,"tx_task1");

	while (1) {
		color_printf(COLOR_PRINT_BLUE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
		color_printf(COLOR_PRINT_BLUE,"tx_task1 notify %d",txpos);
		xEventGroupSetBits(demo_eventgroup, TX1_BIT);
		vTaskDelay(10000 / portTICK_RATE_MS); // delay 10s
		txpos++;
	}

}


void tx_task2(void *arg) {
	uint32_t txpos=0;

	color_printf(COLOR_PRINT_CYAN,"\ttx_task2");

	while (1) {
		color_printf(COLOR_PRINT_CYAN,"\tfree DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
		color_printf(COLOR_PRINT_CYAN,"\ttx_task2 notify %d",txpos);
		xEventGroupSetBits(demo_eventgroup, TX2_BIT);
		vTaskDelay(7000 / portTICK_RATE_MS); // delay 7s
		txpos++;
	}

}

void rx_task(void *arg) {
	uint32_t rxpos=0;
	EventBits_t bits;

	color_printf(COLOR_PRINT_GREEN,"\t\trx_task");

	while (1) {
		color_printf(COLOR_PRINT_GREEN,"\t\trx_task eventgroup yield");
		// read Bits and clear
		bits=xEventGroupWaitBits(demo_eventgroup, TX1_BIT|TX2_BIT,pdTRUE, pdFALSE, 60000 / portTICK_RATE_MS); // max wait 60s
		if(bits==0) {  // xWaitForAllBits == pdFALSE, so we wait for TX1_BIT or TX2_BIT so 0 is timeout
			color_printf(COLOR_PRINT_RED,"\t\tfail to receive eventgroup value");
		} else {
			color_printf(COLOR_PRINT_GREEN,"\t\tfree DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
			color_printf(COLOR_PRINT_GREEN,"\t\trx_task get eventgroup from %s (%d) %d",bits==TX1_BIT?"task 1":(bits==TX2_BIT?"task 2":"both tasks"),bits,rxpos);
		}
		rxpos++;
	}
}

void rx_sync_task(void *arg) {
	uint32_t rxpos=0;
	EventBits_t bits;

	color_printf(COLOR_PRINT_GREEN,"\t\trx_sync_task");

	while (1) {
		color_printf(COLOR_PRINT_GREEN,"\t\trx_sync_task eventgroup yield");
		// read Bits and clear
		bits=xEventGroupWaitBits(demo_eventgroup, TX1_BIT|TX2_BIT,pdTRUE, pdTRUE, 60000 / portTICK_RATE_MS); // max wait 60s
		if(bits!=(TX1_BIT|TX2_BIT)) {  // xWaitForAllBits == pdTRUE, so we wait for TX1_BIT and TX2_BIT so all other is timeout
			color_printf(COLOR_PRINT_RED,"\t\tfail to receive eventgroup value");
		} else {
			color_printf(COLOR_PRINT_GREEN,"\t\tfree DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
			color_printf(COLOR_PRINT_GREEN,"\t\trx_sync_task get eventgroup from %s (%d) %d",bits==TX1_BIT?"task 1":(bits==TX2_BIT?"task 2":"both tasks"),bits,rxpos);
		}
		rxpos++;
	}
}

void app_main() {
	color_printf(COLOR_PRINT_PURPLE,"start ESP32");
	color_printf(COLOR_PRINT_PURPLE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));

	demo_eventgroup = xEventGroupCreate();

	color_printf(COLOR_PRINT_PURPLE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));

	color_printf(COLOR_PRINT_PURPLE,"create three tasks");
    xTaskCreate(tx_task1, "tx_task1", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(tx_task2, "tx_task2", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);
//    xTaskCreate(rx_task, "rx_task", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(rx_sync_task, "rx_sync_task", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);

    color_printf(COLOR_PRINT_PURPLE,"end of main");
}
