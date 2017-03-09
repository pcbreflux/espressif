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

const int GROUPSYNC1_BIT = BIT0;
const int GROUPSYNC2_BIT = BIT1;
const int GROUPSYNC3_BIT = BIT2;

const int ALL_SYNC_BITS = ( BIT0 | BIT1 | BIT2 );

#define COLOR_PRINT_BLACK   "30"
#define COLOR_PRINT_RED     "31"
#define COLOR_PRINT_GREEN   "32"
#define COLOR_PRINT_BROWN   "33"
#define COLOR_PRINT_BLUE    "34"
#define COLOR_PRINT_PURPLE  "35"
#define COLOR_PRINT_CYAN    "36"
#define color_printf(COLOR,format, ... ) { printf("\033[0;" COLOR "m" format "\033[0m\n", ##__VA_ARGS__); }

void groupsync_task1(void *arg) {
	uint32_t syncpos=0;
	EventBits_t bits;

	color_printf(COLOR_PRINT_BLUE,"\tgroupsync_task1");

	while (1) {
		vTaskDelay(5000 / portTICK_RATE_MS); // delay 5s
		color_printf(COLOR_PRINT_BLUE,"\tgroupsync_task1 eventgroup synct yield");
		// read Bits and clear
		bits=xEventGroupSync(demo_eventgroup, GROUPSYNC1_BIT, ALL_SYNC_BITS, 60000 / portTICK_RATE_MS); // max wait 60s
		if(bits!=ALL_SYNC_BITS) {  // xWaitForAllBits == pdTRUE, so we wait for TX1_BIT and TX2_BIT so all other is timeout
			color_printf(COLOR_PRINT_RED,"\tfail to receive synct eventgroup value");
		} else {
			color_printf(COLOR_PRINT_BLUE,"\tfree DRAM %u IRAM %u HWM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT),uxTaskGetStackHighWaterMark(NULL));
			color_printf(COLOR_PRINT_BLUE,"\tgroupsync_task1 get sync eventgroup from all tasks %d",syncpos);
		}
		syncpos++;
	}
}

void groupsync_task2(void *arg) {
	uint32_t syncpos=0;
	EventBits_t bits;

	color_printf(COLOR_PRINT_CYAN,"\t\tgroupsync_task2");

	while (1) {
		vTaskDelay(7000 / portTICK_RATE_MS); // delay 7s
		color_printf(COLOR_PRINT_CYAN,"\t\tgroupsync_task2 eventgroup synct yield");
		// read Bits and clear
		bits=xEventGroupSync(demo_eventgroup, GROUPSYNC2_BIT, ALL_SYNC_BITS, 60000 / portTICK_RATE_MS); // max wait 60s
		if(bits!=ALL_SYNC_BITS) {  // xWaitForAllBits == pdTRUE, so we wait for TX1_BIT and TX2_BIT so all other is timeout
			color_printf(COLOR_PRINT_RED,"\t\tfail to receive synct eventgroup value");
		} else {
			color_printf(COLOR_PRINT_CYAN,"\t\tfree DRAM %u IRAM %u HWM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT),uxTaskGetStackHighWaterMark(NULL));
			color_printf(COLOR_PRINT_CYAN,"\t\tgroupsync_task2 get sync eventgroup from all tasks %d",syncpos);
		}
		syncpos++;
	}
}

void groupsync_task3(void *arg) {
	uint32_t syncpos=0;
	EventBits_t bits;

	color_printf(COLOR_PRINT_GREEN,"\t\t\tgroupsync_task3");

	while (1) {
		vTaskDelay(10000 / portTICK_RATE_MS); // delay 10s
		color_printf(COLOR_PRINT_GREEN,"\t\t\tgroupsync_task3 eventgroup synct yield");
		// read Bits and clear
		bits=xEventGroupSync(demo_eventgroup, GROUPSYNC3_BIT, ALL_SYNC_BITS, 60000 / portTICK_RATE_MS); // max wait 60s
		if(bits!=ALL_SYNC_BITS) {  // xWaitForAllBits == pdTRUE, so we wait for TX1_BIT and TX2_BIT so all other is timeout
			color_printf(COLOR_PRINT_RED,"\t\t\tfail to receive synct eventgroup value");
		} else {
			color_printf(COLOR_PRINT_GREEN,"\t\t\tfree DRAM %u IRAM %u HWM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT),uxTaskGetStackHighWaterMark(NULL));
			color_printf(COLOR_PRINT_GREEN,"\t\t\tgroupsync_task3 get sync eventgroup from all tasks %d",syncpos);
		}
		syncpos++;
	}
}

void app_main() {
	color_printf(COLOR_PRINT_PURPLE,"start ESP32");
	color_printf(COLOR_PRINT_PURPLE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));

	demo_eventgroup = xEventGroupCreate();

	color_printf(COLOR_PRINT_PURPLE,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));

	color_printf(COLOR_PRINT_PURPLE,"create three tasks");
    xTaskCreate(groupsync_task1, "groupsync_task1", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(groupsync_task2, "groupsync_task2", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(groupsync_task3, "groupsync_task3", CONFIG_SYSTEM_EVENT_TASK_STACK_SIZE, NULL, 5, NULL);

    color_printf(COLOR_PRINT_PURPLE,"end of main");
}
