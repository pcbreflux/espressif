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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "smart_led.h"
#include "apa102.h"

static const char *TAG = "SMART_LED";

static apa102 oAPA102(MAX_COLORS);

void set_smart_led(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness) {
	apa102::colorRGBB color;
	color.red=red;
	color.green=green;
	color.blue=blue;
	color.brightness=brightness;
	ESP_LOGI(TAG, "APA102 setColor: %d,%d,%d,%d",red, green, blue, brightness);
	if (brightness==255) {
		oAPA102.test();
		oAPA102.test2();
	} else {
		oAPA102.setColor(color);
	}
}



