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

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "u8g2.h"

#include "sdkconfig.h"

#include "u8g2_esp32_hal.h"
#include "oled_SSD1306.h"

#define PIN_SDA 21
#define PIN_SCL 22

#define GPIO_HALL_EFFECT_SENSOR     13

#define TAG "OLED"

EventGroupHandle_t alarm_eventgroup;

const int HALL_SENSOR_BIT = BIT0;

void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    BaseType_t xHigherPriorityTaskWoken;
    if (gpio_num==GPIO_HALL_EFFECT_SENSOR) {
    	xEventGroupSetBitsFromISR(alarm_eventgroup, HALL_SENSOR_BIT, &xHigherPriorityTaskWoken);
    }
}

void printAlarm(u8g2_t *u8g2,uint32_t loop) {
 	char buf[256];
 	EventBits_t bits = 0;

	u8g2_ClearBuffer(u8g2);

	bits=xEventGroupGetBits(alarm_eventgroup);


	//u8g2_SetFont(u8g2,u8g2_font_ncenB14_tr);
	u8g2_SetFont(u8g2,u8g2_font_fur20_tr);
	if ((bits&HALL_SENSOR_BIT)==1) {
		sprintf(buf,"Alarm");
	} else {
		if (loop%2==0) { // Heartbeat
		    u8g2_DrawDisc(u8g2,122,5,3,U8G2_DRAW_ALL);
		}
		sprintf(buf,"Standby");
	}
	u8g2_DrawStr(u8g2,2,22,buf);

	u8g2_SendBuffer(u8g2);
}


void init_gpio(void) {
    gpio_config_t io_conf;

    //interrupt of falling edge
    io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = (1<<GPIO_HALL_EFFECT_SENSOR);
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

    //install gpio isr service
    gpio_install_isr_service(0); // no flags
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_HALL_EFFECT_SENSOR, gpio_isr_handler, (void*) GPIO_HALL_EFFECT_SENSOR);

}

void oled_hall_task(void *pvParameters) {
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda   = PIN_SDA;
	u8g2_esp32_hal.scl  = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

    alarm_eventgroup = xEventGroupCreate();
    init_gpio();

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
		printAlarm(&u8g2,loop);
		vTaskDelay(1000 / portTICK_RATE_MS); // sleep 1s
		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
