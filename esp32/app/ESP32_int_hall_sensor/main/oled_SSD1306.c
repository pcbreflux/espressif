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
#include <time.h>
#include <sys/time.h>

#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/adc.h"
#include "driver/spi_master.h"
#include "u8g2.h"

#include "sdkconfig.h"

#include "u8g2_esp32_hal.h"
#include "oled_SSD1306.h"

#define PIN_SDA 21
#define PIN_SCL 22

#define GPIO_HALL_EFFECT_SENSOR     13

#define TAG "OLED"

void printValue(u8g2_t *u8g2,uint32_t loop) {
 	char buf[256];
 	int value;

 	value = hall_sensor_read();

 	u8g2_ClearBuffer(u8g2);

	//u8g2_SetFont(u8g2,u8g2_font_ncenB14_tr);
	u8g2_SetFont(u8g2,u8g2_font_fur20_tr);
	sprintf(buf,"%d",value);
	u8g2_DrawStr(u8g2,2,22,buf);

	u8g2_DrawFrame(u8g2,0,28,122,4);
	u8g2_DrawBox(u8g2,(value/6)+44,29,4,2);
	u8g2_SendBuffer(u8g2);
}


void init_adc(void) {
	adc1_config_width(ADC_WIDTH_12Bit); // The Hall Sensor uses channels 0 and 3 of ADC1
	adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_0db);
	adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_0db);
}

void oled_hall_task(void *pvParameters) {
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda   = PIN_SDA;
	u8g2_esp32_hal.scl  = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

    init_adc();

	u8g2_t u8g2; // a structure which will contain all the data for one display
	// u8g2_Setup_ssd1306_128x64_noname_f(
	u8g2_Setup_ssd1306_128x32_univision_f(
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
		printValue(&u8g2,loop);
		vTaskDelay(100 / portTICK_RATE_MS); // sleep 100ms
		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
