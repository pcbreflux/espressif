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

#include "sys/time.h"

#include "sdkconfig.h"

#include "esp_deep_sleep.h"

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 10 seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory

void app_main() {
	struct timeval now;

	printf("start ESP32\n");

	gettimeofday(&now, NULL);

	printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec-last);

	last = now.tv_sec;
	esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);

}

