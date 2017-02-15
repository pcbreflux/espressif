/* Copyright (c) 2016 pcbreflux. All Rights Reserved.
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

extern "C"
{
	#include "sdkconfig.h"

	#include "esp_log.h"
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"

	void app_main();

}
#include "cpptest.h"

void app_main() {
	uint32_t pos=0;
	cpptest oCppTest(1,2);

	printf("hello ESP32\n");
	while (true) {
		printf("main loop %u,%u\n\r",oCppTest.getTx(),oCppTest.getTy());
		printf("tick (%u)\n\r",pos++);
		oCppTest.next();
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
