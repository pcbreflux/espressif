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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "sdkconfig.h"

#include "WS2812.h"
#include "ws2812_task.h"


#define TAG "WS2812_TASK"

#define WS2812_GIPO GPIO_NUM_22
#define WS2812_PIXEL_COUNT 16

static WS2812 *oWS2812;

uint8_t random(uint8_t min,uint8_t max) {
	if (min>max) {
		uint8_t swap;
		swap = min;
		min = max;
		max = swap;
	}
	return (uint8_t)(min + esp_random() % (max + 1 - min));
}


void ramdomBlink(uint16_t pixelCount, uint32_t loops, uint16_t delayms) {

	ESP_LOGI(TAG, "ramdomBlink %d",loops);
	uint8_t bpos,bdir;

	bpos=0;
	bdir=0;
	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < pixelCount; i++) {

			oWS2812->setPixel(i,random(0,255),random(0,255),random(0,255));
		}
		if (bdir==0) {
			bpos++;
		} else {
			bpos--;
		}
		if (bpos>=31) {
			bpos=30;
			bdir=1;
		}
		if (bpos==0) {
			bpos=1;
			bdir=0;
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}

void setColor(uint16_t pixelCount, pixel_t pixel) {

	for(uint16_t i = 0; i < pixelCount; i++) {
		oWS2812->setPixel(i,pixel.red,pixel.green,pixel.blue);
	}
	oWS2812->show();
}

void setRainbow(uint16_t pixelCount, uint32_t loops, uint16_t delayms) {
	double dHue;
	uint16_t hue;

	ESP_LOGI(TAG, "setRainbow %d",loops);
	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			dHue = (double)360/pixelCount*(pos+i);
			hue = (uint16_t)dHue % 360;
			oWS2812->setHSBPixel(i, hue, 255, 127);
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
}

void fadingRainbow(uint16_t pixelCount, uint16_t delayms) {
	double dHue;
	uint16_t hue;

	ESP_LOGI(TAG, "fadingRainbow %d",pixelCount);
	for(uint32_t pos = 0; pos < 31; pos++) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			dHue = (double)360/pixelCount*(pos+i);
			hue = (uint16_t)dHue % 360;
			oWS2812->setHSBPixel(i, hue, 255, pos*4);
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
	for(uint32_t pos = 30; pos > 0; pos--) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			dHue = (double)360/pixelCount*(pos+i);
			hue = (uint16_t)dHue % 360;
			oWS2812->setHSBPixel(i, hue, 255, pos*4);
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
}

void bluringRainbow(uint16_t pixelCount, uint16_t delayms) {
	double dHue;
	uint16_t hue;

	ESP_LOGI(TAG, "bluringRainbow %d",pixelCount);
	for(uint32_t pos = 0; pos < 31; pos++) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			dHue = (double)360/pixelCount*(pos+i);
			hue = (uint16_t)dHue % 360;
			oWS2812->setHSBPixel(i, hue, pos*8, pos*4);
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
	for(uint32_t pos = 30; pos > 0; pos--) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			dHue = (double)360/pixelCount*(pos+i);
			hue = (uint16_t)dHue % 360;
			oWS2812->setHSBPixel(i, hue, pos*8, pos*4);
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
}

void bluringfullRainbow(uint16_t pixelCount, uint16_t delayms) {
	double dHue;
	uint16_t hue;

	ESP_LOGI(TAG, "bluringfullRainbow %d",pixelCount);
	for(uint32_t pos = 0; pos < 127; pos++) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			dHue = (double)360/127*(pos+i);
			hue = (uint16_t)dHue % 360;
			oWS2812->setHSBPixel(i, hue, pos*2, pos);
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
	for(uint32_t pos = 126; pos > 0; pos--) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			dHue = (double)360/127*(pos+i);
			hue = (uint16_t)dHue % 360;
			oWS2812->setHSBPixel(i, hue, pos*2, pos);
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
}

void fadeInOutColor(uint16_t pixelCount, pixel_t pixel, uint16_t delayms) {

	ESP_LOGI(TAG, "fadeInOutColor %d",pixelCount);
	pixel_t cur_pixel;

	for(uint32_t pos = 0; pos < 31; pos++) {
		cur_pixel.red = (uint8_t)((uint32_t)pixel.red*pos/32);
		cur_pixel.green = (uint8_t)((uint32_t)pixel.green*pos/32);
		cur_pixel.blue = (uint8_t)((uint32_t)pixel.blue*pos/32);

		setColor(pixelCount,cur_pixel);
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
	for(uint32_t pos = 30; pos > 0; pos--) {
		cur_pixel.red = (uint8_t)((uint32_t)pixel.red*pos/32);
		cur_pixel.green = (uint8_t)((uint32_t)pixel.green*pos/32);
		cur_pixel.blue = (uint8_t)((uint32_t)pixel.blue*pos/32);

		setColor(pixelCount,cur_pixel);
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
}

void ramdomWalk(uint16_t pixelCount, uint32_t loops, uint16_t delayms) {

	ESP_LOGI(TAG, "ramdomWalk %d",loops);
	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			if (i==(pos%(pixelCount))) {
				oWS2812->setPixel(i,random(0,255),random(0,255),random(0,255));
			} else {
				oWS2812->setPixel(i,0,0,0);
			}
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}

void ramdomBackWalk(uint16_t pixelCount, uint32_t loops, uint16_t delayms) {

	ESP_LOGI(TAG, "ramdomBackWalk %d",loops);
	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < pixelCount; i++) {
			if (i==((pixelCount)-(pos%(pixelCount)))) {
				oWS2812->setPixel(i,random(0,255),random(0,255),random(0,255));
			} else {
				oWS2812->setPixel(i,0,0,0);
			}
		}
		oWS2812->show();
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
}


void ws2812_task(void *pvParameters) {
	pixel_t pixel;
	uint32_t looppos=0;
	uint16_t delayms=25;
	oWS2812 = new WS2812(WS2812_GIPO,WS2812_PIXEL_COUNT,0);

	while(1) {
			ESP_LOGI(TAG, "test sequence %d",looppos);

			bluringRainbow(WS2812_PIXEL_COUNT, 20);

			ramdomBlink(WS2812_PIXEL_COUNT,10,100);

			setRainbow(WS2812_PIXEL_COUNT,WS2812_PIXEL_COUNT, 100);

			pixel.red=255;
			pixel.green=0;
			pixel.blue=0;
			fadeInOutColor(WS2812_PIXEL_COUNT,pixel,delayms);

			pixel.red=0;
			pixel.green=255;
			pixel.blue=0;
			fadeInOutColor(WS2812_PIXEL_COUNT,pixel,delayms);

			pixel.red=0;
			pixel.green=0;
			pixel.blue=255;
			fadeInOutColor(WS2812_PIXEL_COUNT,pixel,delayms);

			pixel.red=0;
			pixel.green=255;
			pixel.blue=255;
			fadeInOutColor(WS2812_PIXEL_COUNT,pixel,delayms);

			pixel.red=255;
			pixel.green=0;
			pixel.blue=255;
			fadeInOutColor(WS2812_PIXEL_COUNT,pixel,delayms);

			pixel.red=255;
			pixel.green=255;
			pixel.blue=0;
			fadeInOutColor(WS2812_PIXEL_COUNT,pixel,delayms);

			pixel.red=255;
			pixel.green=255;
			pixel.blue=255;
			fadeInOutColor(WS2812_PIXEL_COUNT,pixel,delayms);

			fadingRainbow(WS2812_PIXEL_COUNT, 100);

			ramdomWalk(WS2812_PIXEL_COUNT,WS2812_PIXEL_COUNT,10);
			ramdomBackWalk(WS2812_PIXEL_COUNT,WS2812_PIXEL_COUNT,10);
			ramdomWalk(WS2812_PIXEL_COUNT,WS2812_PIXEL_COUNT,30);
			ramdomBackWalk(WS2812_PIXEL_COUNT,WS2812_PIXEL_COUNT,30);
			ramdomWalk(WS2812_PIXEL_COUNT,WS2812_PIXEL_COUNT,10);
			ramdomBackWalk(WS2812_PIXEL_COUNT,WS2812_PIXEL_COUNT,10);

			bluringfullRainbow(WS2812_PIXEL_COUNT, 50);

			looppos++;
		}
	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
