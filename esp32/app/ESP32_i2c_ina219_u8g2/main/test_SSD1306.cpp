#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

#include "INA219.h"

extern "C"
{
#include "u8g2_esp32_hal.h"

void task_test_SSD1306(void *ignore);
}


#define PIN_SDA GPIO_NUM_21
#define PIN_SCL GPIO_NUM_22


static const char *TAG = "ssd1306";


void task_test_SSD1306(void *ignore) {
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda   = PIN_SDA;
	u8g2_esp32_hal.scl  = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);
	float shuntvoltage = 0;
	float busvoltage = 0;
	float current_mA = 0;
	float loadvoltage = 0;
	float power = 0;

	INA219 ina219;


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

	int pos = 0;
	uint32_t loop=0;
	char buf[256];

	u8g2_ClearBuffer(&u8g2);
	u8g2_SendBuffer(&u8g2);

    ina219.begin();

	while (1) {
		pos = 0;
		while (pos<128) {
			shuntvoltage = ina219.shuntVoltage()*1000;
			busvoltage = ina219.busVoltage();
			current_mA = ina219.shuntCurrent()*1000;
			loadvoltage = busvoltage + (shuntvoltage/1000);
			power = ina219.busPower()*1000;

			ESP_LOGD(TAG, "u8g2_ClearBuffer");
			u8g2_ClearBuffer(&u8g2);
			ESP_LOGD(TAG, "u8g2_DrawBox");
			u8g2_DrawBox(&u8g2, 0, 10, pos,4);
			u8g2_DrawFrame(&u8g2, 0,10,127,4);

			ESP_LOGD(TAG, "u8g2_SetFont");
			u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
			ESP_LOGD(TAG, "u8g2_DrawStr");
			sprintf(buf,"L%u P%d",loop,pos);
			u8g2_DrawStr(&u8g2, 2,8,buf);

			sprintf(buf,"V- %f V",busvoltage);
			u8g2_DrawStr(&u8g2, 2,26,buf);
			sprintf(buf,"dV %f mV",shuntvoltage);
			u8g2_DrawStr(&u8g2, 2,34,buf);
			sprintf(buf,"V+ %f V",loadvoltage);
			u8g2_DrawStr(&u8g2, 2,42,buf);
			sprintf(buf,"I  %f mA",current_mA);
			u8g2_DrawStr(&u8g2, 2,50,buf);
			sprintf(buf,"P  %f mW",power);
			u8g2_DrawStr(&u8g2, 2,58,buf);

			ESP_LOGD(TAG, "u8g2_SendBuffer");
			u8g2_SendBuffer(&u8g2);
			vTaskDelay(1/portTICK_PERIOD_MS);
			pos++;
		}
		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}

