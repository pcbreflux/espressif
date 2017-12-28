/** @file test_GDEP015OC1.c
 *
 * Copyright (c) 2017 pcbreflux. All Rights Reserved.
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
 *
 * @brief Application main task file with testcode for one display.
 *
*/
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <u8g2.h>

#include "sdkconfig.h"
#include "u8g2_esp32_hal.h"

#define EPAPER_GIPO_MOSI GPIO_NUM_23 // sda
#define EPAPER_GIPO_SCK GPIO_NUM_18 // sclk
#define EPAPER_GIPO_CS GPIO_NUM_5 // nCS
#define EPAPER_GIPO_DC GPIO_NUM_21 // nDC
#define EPAPER_GIPO_RESET GPIO_NUM_17 // nRES
#define EPAPER_GIPO_BUSY GPIO_NUM_16 // BUSY


static const char *TAG = "gdep015oc1";

const short sintab60[60] = {
    // round(sin(2*pi/60 * 0:59) * 100)
	0,10,21,31,41,50,59,67,74,81,87,91,95,98,99,100,99,98,95,91,87,81,
	74,67,59,50,41,31,21,10,0,-10,-21,-31,-41,-50,-59,-67,-74,-81,-87,
	-91,-95,-98,-99,-100,-99,-98,-95,-91,-87,-81,-74,-67,-59,-50,-41,
	-31,-21,-10
};

const uint8_t chess_black_queen[] =
{
  /* QUEEN */		0x00, 0x55, 0x3e, 0x3e, 0x1c, 0x3e, 0x1c, 0x7f
};

const uint8_t bat32x16 [] = {
0X00, 0X00, 0X00, 0X00, 0X1F, 0XFF, 0XFF, 0XFC, 0X3F, 0XFF, 0XFF, 0XFE, 0X30, 0X00, 0X00, 0X06,
0X70, 0X19, 0XCE, 0X66, 0X70, 0X19, 0XCE, 0X66, 0X70, 0X19, 0XCE, 0X66, 0X70, 0X19, 0XCE, 0X66,
0X70, 0X19, 0XCE, 0X66, 0X70, 0X19, 0XCE, 0X66, 0X70, 0X19, 0XCE, 0X66, 0X70, 0X19, 0XCE, 0X66,
0X30, 0X00, 0X00, 0X06, 0X3F, 0XFF, 0XFF, 0XFE, 0X1F, 0XFF, 0XFF, 0XFC, 0X00, 0X00, 0X00, 0X00
};

const uint8_t queen32x64 [] = {
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0XC0, 0X00,
0X00, 0X07, 0XE0, 0X00, 0X00, 0X07, 0XF0, 0X00, 0X00, 0X07, 0XF0, 0X00, 0X00, 0X07, 0XF0, 0X00,
0X00, 0X07, 0XE0, 0X00, 0X00, 0X01, 0XC0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0XCE, 0X31, 0X80,
0X03, 0XFF, 0XFF, 0XC0, 0X01, 0XFF, 0XFF, 0XC0, 0X00, 0XFF, 0XFF, 0X00, 0X00, 0X7F, 0XFF, 0X00,
0X00, 0X7F, 0XFE, 0X00, 0X00, 0X3F, 0XFC, 0X00, 0X00, 0X1F, 0XF8, 0X00, 0X00, 0X1F, 0XF8, 0X00,
0X00, 0X1F, 0XF8, 0X00, 0X00, 0X0F, 0XF8, 0X00, 0X00, 0X0F, 0XF0, 0X00, 0X00, 0X0F, 0XF8, 0X00,
0X00, 0X0F, 0XF0, 0X00, 0X00, 0X0F, 0XF0, 0X00, 0X00, 0X7F, 0XFC, 0X00, 0X00, 0XFF, 0XFF, 0X00,
0X00, 0XFF, 0XFE, 0X00, 0X00, 0X05, 0X50, 0X00, 0X00, 0X07, 0XF0, 0X00, 0X00, 0X07, 0XF0, 0X00,
0X00, 0X0F, 0XF0, 0X00, 0X00, 0X0F, 0XF0, 0X00, 0X00, 0X0F, 0XF0, 0X00, 0X00, 0X0F, 0XF8, 0X00,
0X00, 0X0F, 0XF0, 0X00, 0X00, 0X0F, 0XF8, 0X00, 0X00, 0X1F, 0XF8, 0X00, 0X00, 0X1F, 0XF8, 0X00,
0X00, 0X1F, 0XFC, 0X00, 0X00, 0X3F, 0XFC, 0X00, 0X00, 0X3F, 0XFC, 0X00, 0X00, 0X7F, 0XFE, 0X00,
0X00, 0X7F, 0XFE, 0X00, 0X00, 0XFF, 0XFF, 0X00, 0X00, 0XFF, 0XFF, 0X00, 0X00, 0X90, 0X00, 0X00,
0X03, 0XFF, 0XFF, 0XC0, 0X01, 0X55, 0X52, 0X00, 0X03, 0XFF, 0XFF, 0XC0, 0X07, 0XFF, 0XFF, 0XC0,
0X07, 0XFF, 0XFF, 0XE0, 0X0F, 0XFF, 0XFF, 0XE0, 0X0F, 0XFF, 0XFF, 0XE0, 0X0F, 0XFF, 0XFF, 0XF0,
0X0F, 0XFF, 0XFF, 0XE0, 0X07, 0XFF, 0XFF, 0XE0, 0X07, 0XFF, 0XFF, 0XE0, 0X01, 0X52, 0X00, 0X00,
0X07, 0XFF, 0XFF, 0XE0, 0X02, 0XAA, 0X92, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00
};

const uint8_t queen64x32 [] = {
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0F, 0XE8,
0X00, 0X0C, 0X00, 0X00, 0X00, 0X00, 0XBF, 0XE8, 0X00, 0X1C, 0X00, 0X00, 0X00, 0X00, 0XBF, 0XE8,
0X00, 0X1F, 0X00, 0X10, 0X00, 0X06, 0XBF, 0XE8, 0X00, 0X0F, 0X80, 0X18, 0X00, 0X1E, 0XFF, 0XEC,
0X00, 0X0F, 0XC0, 0X38, 0X00, 0XFE, 0XBF, 0XE8, 0X00, 0X0F, 0XFD, 0X38, 0X17, 0XFE, 0XBF, 0XE8,
0X07, 0X1F, 0XFF, 0XFF, 0XFF, 0XFE, 0XFF, 0XEC, 0X0F, 0X9F, 0XFF, 0XFB, 0XFF, 0XFE, 0XBF, 0XE8,
0X1F, 0XCF, 0XFF, 0XFF, 0XFF, 0XFE, 0XFF, 0XE8, 0X1F, 0XCF, 0XFF, 0XFB, 0XFF, 0XFE, 0XBF, 0XEC,
0X1F, 0XCF, 0XFF, 0XFF, 0XFF, 0XFE, 0XFF, 0XE8, 0X0F, 0X9F, 0XFF, 0XFB, 0XFF, 0XFE, 0XBF, 0XFC,
0X0F, 0X9F, 0XFF, 0XFF, 0XFF, 0XFE, 0XFF, 0XE8, 0X00, 0X1F, 0XFF, 0XF8, 0XFF, 0XFE, 0XBF, 0XEC,
0X00, 0X0F, 0XF8, 0X38, 0X03, 0XFF, 0XFF, 0XF8, 0X00, 0X0F, 0XC0, 0X38, 0X00, 0X7E, 0XBF, 0XEC,
0X00, 0X1F, 0X80, 0X38, 0X00, 0X1E, 0XFF, 0XF8, 0X00, 0X1E, 0X00, 0X18, 0X00, 0X07, 0XBF, 0XEC,
0X00, 0X1C, 0X00, 0X00, 0X00, 0X00, 0XFF, 0XF8, 0X00, 0X08, 0X00, 0X00, 0X00, 0X00, 0XBF, 0XEC,
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1F, 0XE8, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X07, 0X80,
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00
};

int absrot60(int angle)
{
    while (angle < 0) { angle += 60; }
    while (angle >= 60) { angle -= 60; }
    return angle;
}


void task_test_gdep015oc1(void *ignore) {
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.mosi   = EPAPER_GIPO_MOSI;
	u8g2_esp32_hal.clk  = EPAPER_GIPO_SCK;
	u8g2_esp32_hal.cs  = EPAPER_GIPO_CS;
	u8g2_esp32_hal.dc  = EPAPER_GIPO_DC;
	u8g2_esp32_hal.reset  = EPAPER_GIPO_RESET;
	u8g2_esp32_hal_init(u8g2_esp32_hal);


	u8g2_t u8g2; // a structure which will contain all the data for one display
	u8g2_Setup_gdep015oc1_200x200_f(
		&u8g2,
		U8G2_R0,
		//u8x8_byte_sw_i2c,
		u8g2_esp32_msg_comms_cb,
		u8g2_esp32_msg_gpio_and_delay_cb);  // init u8g2 structure

	ESP_LOGI(TAG, "u8g2_InitDisplay");
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,

	ESP_LOGI(TAG, "u8g2_SetPowerSave");
	u8g2_SetPowerSave(&u8g2, 0); // wake up display

	//int pos = 0;
	uint32_t loop=0;
	char buf[256];

	while (1) {
		ESP_LOGI(TAG, "u8g2_ClearBuffer");
		u8g2_ClearBuffer(&u8g2);

/*
		ESP_LOGI(TAG, "u8g2_DrawHLine");
		//u8g2_DrawLine(&u8g2, 0, 0, 8,0);
		u8g2_DrawHLine(&u8g2, 0, 0, 8);
		u8g2_DrawHLine(&u8g2, 0, 7, 8);
		u8g2_DrawHLine(&u8g2, 0, 8, 8);
		u8g2_DrawHLine(&u8g2, 0, 120, 8);
		u8g2_DrawHLine(&u8g2, 0, 121, 8);
		u8g2_DrawHLine(&u8g2, 0, 122, 8);
		u8g2_DrawHLine(&u8g2, 0, 123, 8);
		u8g2_DrawHLine(&u8g2, 8, 9, 199);

		u8g2_DrawHLine(&u8g2, 199, 199, 8);
*/

	/*
		ESP_LOGI(TAG, "u8g2_DrawHLine");
		//u8g2_DrawLine(&u8g2, 0, 0, 8,0);
		u8g2_DrawVLine(&u8g2, 0, 0, 8);
		u8g2_DrawVLine(&u8g2, 7, 0, 8);

		for (uint8_t xp=0;xp<10;xp+=2) {
			u8g2_DrawVLine(&u8g2, xp, 0, 199);
		}
//		for (uint8_t xp=220;xp<244;xp+=8) {
//			u8g2_DrawVLine(&u8g2, xp, 0, 122);
//		}
//		u8g2_DrawVLine(&u8g2, 249, 0, 122);

		for (uint8_t xp=140;xp<200;xp+=2) {
			u8g2_DrawVLine(&u8g2, xp, 0, 199);
		}
*/
/*
		if (loop%4==1) {
			u8g2_DrawBox(&u8g2, 0, 0, 199, 199);
		} else if (loop%4==2) {
			for (uint8_t xp=0;xp<200;xp+=2) {
				u8g2_DrawVLine(&u8g2, xp, 0, 199);
			}
		} else if (loop%4==3) {
			for (uint8_t yp=0;yp<200;yp+=2) {
				u8g2_DrawHLine(&u8g2, 0, yp, 199);
			}

		}
*/

		ESP_LOGD(TAG, "u8g2_SetFont");
		u8g2_SetFont(&u8g2, u8g2_font_ncenB24_tr);
		ESP_LOGD(TAG, "u8g2_DrawStr");
		sprintf(buf,"u8g2");
		u8g2_DrawStr(&u8g2, 2,26,buf);
		ESP_LOGD(TAG, "u8g2_DrawStr");
		sprintf(buf,"Demo");
		u8g2_DrawStr(&u8g2, 2,54,buf);

		ESP_LOGD(TAG, "u8g2_SetFont");
		u8g2_SetFont(&u8g2, u8g2_font_helvB14_tr);
		ESP_LOGD(TAG, "u8g2_DrawStr");
		sprintf(buf,"on gdep015oc1");
		u8g2_DrawStr(&u8g2, 2,71,buf);


		ESP_LOGI(TAG, "u8g2_DrawBox");

		u8g2_DrawBox(&u8g2, 1, 110, loop%199,6);
		u8g2_DrawFrame(&u8g2, 1,110,199,6);

		ESP_LOGD(TAG, "u8g2_SetFont");
		u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
		ESP_LOGD(TAG, "u8g2_DrawStr");
		sprintf(buf,"L%u P%d",loop,loop%199);
		u8g2_DrawStr(&u8g2, 2,104,buf);

		for (uint8_t d=2;d<20;d+=4) {
			u8g2_DrawCircle(&u8g2, 60, 160, d,U8G2_DRAW_ALL);
		}
		u8g2_DrawDisc(&u8g2, 100, 160, 20,U8G2_DRAW_ALL);

		u8g2_DrawTriangle(&u8g2, 110, 190, 190, 190, 150,130);
		int center_x = 20;
		int center_y = 160;
		int hours = loop%12;
		int minutes = loop%60;
		int radius=20;

		u8g2_DrawCircle(&u8g2, center_x, center_y, radius,U8G2_DRAW_ALL);

	    int angle = absrot60(-(short)hours*5 + 15);
	    int x2 = sintab60[absrot60(angle+15)]*3/radius + center_x;   // cos, x-axis is natural direction
	    int y2 = (-sintab60[angle])*3/radius + center_y;         // sin, y-axis is inverted

	    u8g2_DrawLine(&u8g2, center_x, center_y, x2, y2);
	    u8g2_DrawLine(&u8g2, center_x+1, center_y, x2+1, y2);
	    u8g2_DrawLine(&u8g2, center_x, center_y+1, x2, y2+1);
	    u8g2_DrawLine(&u8g2, center_x-1, center_y, x2-1, y2);
	    u8g2_DrawLine(&u8g2, center_x, center_y-1, x2, y2-1);

	    angle = absrot60(-(short)minutes + 15);
	    x2 = sintab60[absrot60(angle+15)]*4/radius + center_x;   // cos, x-axis is natural direction
	    y2 = (-sintab60[angle])*4/radius + center_y;         // sin, y-axis is inverted

	    u8g2_DrawLine(&u8g2, center_x, center_y, x2, y2);

	    //u8g2_DrawBitmap(&u8g2, 150, 20, 1, 8, chess_black_queen);
	    u8g2_DrawBitmap(&u8g2, 164, 2, 4, 16, bat32x16); // 32=4x8

	    if (loop%2==0) {
	    	u8g2_DrawBitmap(&u8g2, 164, 20, 4, 64, queen32x64); // 32=4x8
	    } else {
	    	u8g2_DrawBitmap(&u8g2, 132, 20, 8, 32, queen64x32); // 64=8x8

	    }

	    ESP_LOGD(TAG, "u8g2_SendBuffer ptr=0x%08X,cur_row=%d",(unsigned int)u8g2.tile_buf_ptr,u8g2.tile_curr_row);
	    u8g2_SendBuffer(&u8g2);
		vTaskDelay(5000/portTICK_PERIOD_MS);
		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}


