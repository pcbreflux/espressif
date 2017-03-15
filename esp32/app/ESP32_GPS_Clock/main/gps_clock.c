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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/heap_regions.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_heap_alloc_caps.h"

#include "gps_clock.h"

#include "driver/uart.h"
#include "minmea.h"


#define TAG "GPS"

#define GPS_TX_PIN 17
#define GPS_RX_PIN 16
#define GPS_UART UART_NUM_1
#define GPS_UART_BUFFER_SIZE 2048

#define MAX_LINE_SIZE 255

void readLine(uart_port_t uart, uint8_t *line) {
	int size;
	uint8_t *ptr = line;
	while(ptr-line < MAX_LINE_SIZE) {
		size = uart_read_bytes(uart, ptr, 1, portMAX_DELAY);
		if (size == 1) {
			if (*ptr == '\n') {
				ptr++;
				*ptr = 0;
				break;
			}
			ptr++;
		}
	}
}


void initUART(uart_port_t uart) {
	ESP_LOGI(TAG, "init UART");

	uart_config_t myUartConfig;
	myUartConfig.baud_rate           = 9600;
	myUartConfig.data_bits           = UART_DATA_8_BITS;
	myUartConfig.parity              = UART_PARITY_DISABLE;
	myUartConfig.stop_bits           = UART_STOP_BITS_1;
	myUartConfig.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
	myUartConfig.rx_flow_ctrl_thresh = 122;

	uart_param_config(uart, &myUartConfig);

	uart_set_pin(uart,
			GPS_RX_PIN,         // TX
			GPS_TX_PIN,         // RX
			UART_PIN_NO_CHANGE, // RTS
			UART_PIN_NO_CHANGE  // CTS
  );

	uart_driver_install(uart, GPS_UART_BUFFER_SIZE, GPS_UART_BUFFER_SIZE, 0, NULL, 0);
}

void deinitUART(uart_port_t uart) {
	ESP_LOGI(TAG, "deinit UART");

	uart_driver_delete(uart);
}

void gps_clock_task(void *pvParameters) {
	struct timeval tv;
	struct tm mytm;
	uint8_t line[MAX_LINE_SIZE+1];
	uint8_t notimeset=1;

	initUART(GPS_UART);

	while(notimeset) {
		readLine(GPS_UART,line);
		ESP_LOGD(TAG, "%s", line);
		switch(minmea_sentence_id((char *)line, false)) {
		case MINMEA_SENTENCE_RMC:
			ESP_LOGD(TAG, "Sentence - MINMEA_SENTENCE_RMC");
			struct minmea_sentence_rmc senRMC;

			if (minmea_parse_rmc(&senRMC,(char *)line)) {
				ESP_LOGI(TAG, "time: %d:%d:%d %d-%d-%d",
						senRMC.time.hours, senRMC.time.minutes, senRMC.time.seconds,
						2000+senRMC.date.year,senRMC.date.month,senRMC.date.day);
				mytm.tm_hour = senRMC.time.hours;
				mytm.tm_min = senRMC.time.minutes;
				mytm.tm_sec = senRMC.time.seconds;
				mytm.tm_mday = senRMC.date.day;
				mytm.tm_mon = senRMC.date.month-1;
				mytm.tm_year = 100+senRMC.date.year;

			    setenv("TZ", "GMT0GMT0", 1);
			    tzset();
			    time_t t = mktime(&mytm);
				ESP_LOGI(TAG, "time: %ld",t);
				tv.tv_sec = t;
				tv.tv_usec = 0;
		        settimeofday(&tv, NULL); \
		        notimeset=0;
			} else {
				ESP_LOGD(TAG, "$xxRMC sentence is not parsed\n");
			}
			break;
		case MINMEA_SENTENCE_GGA:
			ESP_LOGD(TAG, "Sentence - MINMEA_SENTENCE_GGA");
			break;
		case MINMEA_SENTENCE_GSV:
			//ESP_LOGI(TAG, "Sentence - MINMEA_SENTENCE_GSV");
			break;
		case MINMEA_INVALID:
			ESP_LOGD(TAG, "Sentence - INVALID");
			break;
		default:
			ESP_LOGD(TAG, "Sentence - other");
			break;
		}
	}

	ESP_LOGI(TAG, "Time set!");
	deinitUART(GPS_UART);
	vTaskDelete(NULL);
}
