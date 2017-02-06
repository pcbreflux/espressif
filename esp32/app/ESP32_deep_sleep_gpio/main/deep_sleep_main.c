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

#include "esp_deep_sleep.h"

#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0

void app_main() {
	printf("start ESP32\n");

	printf("config IO\n");
	esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO); //!< Keep power domain enabled in deep sleep, if it is needed by one of the wakeup options. Otherwise power it down.
	gpio_pullup_en(GPIO_INPUT_IO_TRIGGER);		// use pullup on GPIO
	gpio_pulldown_dis(GPIO_INPUT_IO_TRIGGER);       // not use pulldown on GPIO

	esp_deep_sleep_enable_ext0_wakeup(GPIO_INPUT_IO_TRIGGER, 0); // Wake if GPIO is low

	printf("deep sleep\n");
	esp_deep_sleep_start();

}
