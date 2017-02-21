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
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/heap_regions.h"

#define COLOR_PRINT_BLACK   "30"
#define COLOR_PRINT_RED     "31"
#define COLOR_PRINT_GREEN   "32"
#define COLOR_PRINT_BROWN   "33"
#define COLOR_PRINT_BLUE    "34"
#define COLOR_PRINT_PURPLE  "35"
#define COLOR_PRINT_CYAN    "36"
#define color_printf(COLOR,format, ... ) { printf("\033[0;" COLOR "m" format "\033[0m\n", ##__VA_ARGS__); }

SemaphoreHandle_t demo_semaphore1;
SemaphoreHandle_t demo_semaphore2;

// let the rx_task1 wait for us
void tx_task1(void *arg) {
	uint32_t txpos=0;

	color_printf(COLOR_PRINT_BLUE,"tx_task1");
	color_printf(COLOR_PRINT_BLUE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
	demo_semaphore1 = xSemaphoreCreateBinary();

	while (1) {
		vTaskDelay(10000 / portTICK_RATE_MS); // delay 10s
		color_printf(COLOR_PRINT_BLUE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
		color_printf(COLOR_PRINT_BLUE,"tx_task1 notify %d",txpos);
		xSemaphoreGive(demo_semaphore1);
		txpos++;
	}

}

// wait for tx_task1 to give semaphore
void rx_task1(void *arg) {
	uint32_t rxpos=0;

	color_printf(COLOR_PRINT_GREEN,"rx_task1");

	while (1) {
		if (demo_semaphore1!=NULL) {
			color_printf(COLOR_PRINT_GREEN,"rx_task1 semaphore yield");
			if(xSemaphoreTake(demo_semaphore1,60000/portTICK_RATE_MS)!=pdTRUE) {  // max wait 60s
				color_printf(COLOR_PRINT_RED,"rx_task1 fail to receive semaphore");
			} else {
				color_printf(COLOR_PRINT_GREEN,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
				color_printf(COLOR_PRINT_GREEN,"rx_task1 get semaphore %d",rxpos);
			}
		} else {
			vTaskDelay(100 / portTICK_RATE_MS); // delay 100ms
		}
		rxpos++;
	}
}

// let the rx_task2 wait for us and then wait for rx_task2 to finish (give semaphore)
void tx_task2(void *arg) {
	uint32_t txpos=0;

	color_printf(COLOR_PRINT_CYAN,"\ttx_task2");
	color_printf(COLOR_PRINT_CYAN,"\tfree DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
	demo_semaphore2 = xSemaphoreCreateBinary();

	while (1) {
		vTaskDelay(7000 / portTICK_RATE_MS); // delay 7s
		color_printf(COLOR_PRINT_CYAN,"\tfree DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
		color_printf(COLOR_PRINT_CYAN,"\ttx_task2 notify %d",txpos);
		xSemaphoreGive(demo_semaphore2);
		color_printf(COLOR_PRINT_CYAN,"\ttx_task2 semaphore yield");
		if(xSemaphoreTake(demo_semaphore2,60000/portTICK_RATE_MS)!=pdTRUE) {  // max wait 60s
			color_printf(COLOR_PRINT_RED,"\ttx_task2 fail to receive semaphore");
		} else {
			color_printf(COLOR_PRINT_CYAN,"\tfree DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
			color_printf(COLOR_PRINT_CYAN,"\ttx_task2 get semaphore %d",txpos);
		}
		txpos++;
	}

}

// wait for tx_task2 to give semaphore and then let the tx_task2 wait for us
void rx_task2(void *arg) {
	uint32_t rxpos=0;

	color_printf(COLOR_PRINT_BROWN,"\trx_task2");

	while (1) {
		if (demo_semaphore2!=NULL) {
			color_printf(COLOR_PRINT_BROWN,"\trx_task2 semaphore yield");
			if(xSemaphoreTake(demo_semaphore2,60000/portTICK_RATE_MS)!=pdTRUE) {  // max wait 60s
				color_printf(COLOR_PRINT_RED,"\trx_task2 fail to receive semaphore");
			} else {
				color_printf(COLOR_PRINT_BROWN,"\tfree DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
				color_printf(COLOR_PRINT_BROWN,"\trx_task2 get semaphore %d",rxpos);
			}
			vTaskDelay(2000 / portTICK_RATE_MS); // delay 2s
			xSemaphoreGive(demo_semaphore2);
		} else {
			vTaskDelay(100 / portTICK_RATE_MS); // delay 100ms
		}
		rxpos++;
	}
}


void app_main() {
	color_printf(COLOR_PRINT_PURPLE,"start ESP32");
	color_printf(COLOR_PRINT_PURPLE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));

	color_printf(COLOR_PRINT_PURPLE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));

	color_printf(COLOR_PRINT_PURPLE,"create three tasks");
    xTaskCreate(tx_task1, "tx_task1", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(rx_task1, "rx_task1", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(tx_task2, "tx_task2", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(rx_task2, "rx_task2", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);

	color_printf(COLOR_PRINT_PURPLE,"end of main");
}
