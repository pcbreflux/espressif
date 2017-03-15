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
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <u8g2.h>
#include <time.h>
#include <sys/time.h>

#include "sdkconfig.h"

#include "u8g2_esp32_hal.h"
#include "oled_SSD1306.h"
#include "gps_clock.h"

#define PIN_SDA 21
#define PIN_SCL 22

#define TAG "OLED"

void printTime(u8g2_t *u8g2,uint32_t loop) {
    char strftime_buf[64];
	char buf[256];
    time_t now = 0;
    struct tm timeinfo;

    time(&now);

    // Set timezone to Central Europe Standard Time and print local time
    setenv("TZ", "GMT-1GMT-2,M3.5.0/2,M10.5.0", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGD(TAG, "CET DST: %s", strftime_buf);

	u8g2_ClearBuffer(u8g2);

	if (timeinfo.tm_sec%4==0 && timeinfo.tm_year < 71) { // Heartbeat
	    u8g2_DrawDisc(u8g2,122,5,3,U8G2_DRAW_ALL);
	}

	//u8g2_SetFont(u8g2,u8g2_font_ncenB14_tr);
	u8g2_SetFont(u8g2,u8g2_font_fur20_tr);
	sprintf(buf,"%02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
	u8g2_DrawStr(u8g2,2,22,buf);
	u8g2_SetFont(u8g2,u8g2_font_logisoso16_tr);
	sprintf(buf,"%04d-%02d-%02d",timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday);
	u8g2_DrawStr(u8g2,6,42,buf);

	u8g2_DrawFrame(u8g2,0,50,122,14);
	u8g2_DrawBox(u8g2,1,51,(timeinfo.tm_hour%12)*10,4);
	u8g2_DrawBox(u8g2,1,55,(timeinfo.tm_min)*2,4);
	u8g2_DrawBox(u8g2,1,59,(timeinfo.tm_sec)*2,4);

	u8g2_SendBuffer(u8g2);
}
void obtain_gpstime(void) {
    xTaskCreate(gps_clock_task, "gps_clock_task", 4096, NULL, 5, NULL);
}

void oled_clock_task(void *pvParameters) {
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda   = PIN_SDA;
	u8g2_esp32_hal.scl  = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);


	u8g2_t u8g2; // a structure which will contain all the data for one display
	u8g2_Setup_ssd1306_128x64_noname_f(
	//u8g2_Setup_ssd1306_128x32_univision_f(
		&u8g2,
		U8G2_R0,
		//u8x8_byte_sw_i2c,
		u8g2_esp32_msg_i2c_cb,
		u8g2_esp32_msg_i2c_and_delay_cb);  // init u8g2 structure
	u8x8_SetI2CAddress(&u8g2.u8x8,0x78);

	ESP_LOGD(TAG, "u8g2_InitDisplay");
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,

	ESP_LOGD(TAG, "u8g2_SetPowerSave");
	u8g2_SetPowerSave(&u8g2, 0); // wake up display

	uint32_t loop=0;

	while (1) {
		if (loop%600==0) {			// adjust every ten minutes
			obtain_gpstime();
		}
		printTime(&u8g2,loop);
		vTaskDelay(1000 / portTICK_RATE_MS); // sleep 1s
		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
