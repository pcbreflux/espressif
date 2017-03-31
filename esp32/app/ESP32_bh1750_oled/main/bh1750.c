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

#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bh1750.h"

static const char *TAG = "bh1750";

#define I2C_ADDR BH1750_ADDRESS1
#define BH1750_MODE BH1750_CONTINUOUS_HIGH_RES_MODE

#define PIN_SDA 21
#define PIN_SCL 22
#define I2C_MASTER_NUM I2C_NUM_1   /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ   10000     /*!< I2C master clock frequency */
#define ACK_CHECK_EN   0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL    0x0         /*!< I2C ack value */
#define NACK_VAL   0x1         /*!< I2C nack value */

int bh1750_I2C_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);
int bh1750_I2C_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);

void bh1750_reset(void) {
   	ESP_LOGE(TAG, "reset");
	bh1750_I2C_write(I2C_ADDR, BH1750_POWER_ON, NULL, 0);
	bh1750_I2C_write(I2C_ADDR, BH1750_RESET, NULL, 0);
	vTaskDelay(10 / portTICK_RATE_MS); // sleep 10ms
}

float bh1750_read(void) {
	uint8_t buf[32];
	uint8_t mode = BH1750_MODE;
	uint8_t sleepms = 1;
	uint8_t resdiv = 1;
	float luxval=0;
	int ret = -1;

	switch (mode) {
	case BH1750_CONTINUOUS_HIGH_RES_MODE:
	case BH1750_ONE_TIME_HIGH_RES_MODE:
		sleepms=180;
		break;
	case BH1750_CONTINUOUS_HIGH_RES_MODE_2:
	case BH1750_ONE_TIME_HIGH_RES_MODE_2:
		sleepms=180;
		resdiv=2;
		break;
	case BH1750_CONTINUOUS_LOW_RES_MODE:
		sleepms=24;
		break;
	case BH1750_ONE_TIME_LOW_RES_MODE:
		sleepms=50;
		break;
	}

	ret=bh1750_I2C_write(I2C_ADDR, mode, NULL, 0);
    if (ret != ESP_OK) {
    	bh1750_reset();
    	return -1;
    }

	vTaskDelay(sleepms / portTICK_RATE_MS); // sleep ms
	ret=bh1750_I2C_read(I2C_ADDR, 0xFF, buf, 2);
    if (ret != ESP_OK) {
    	bh1750_reset();
    	return 0;
    }
	uint16_t luxraw = (uint16_t)(((uint16_t)(buf[0]<<8))|((uint16_t)buf[1]));
	luxval = (float)luxraw/1.2/resdiv;
	ESP_LOGI(TAG, "sensraw=%u lux=%f", luxraw,luxval);

	return luxval;
}

void bh1750_init(void) {

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
	ESP_LOGI(TAG, "sda_io_num %d", PIN_SDA);
    conf.sda_io_num = PIN_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	ESP_LOGI(TAG, "scl_io_num %d", PIN_SCL);
    conf.scl_io_num = PIN_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	ESP_LOGI(TAG, "clk_speed %d", I2C_MASTER_FREQ_HZ);
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	ESP_LOGI(TAG, "i2c_param_config %d", conf.mode);
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
	ESP_LOGI(TAG, "i2c_driver_install %d", I2C_MASTER_NUM);
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));

}

void bh1750_deinit(void) {
	ESP_LOGI(TAG, "i2c_driver_delete");
	ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));

}

/*	\Brief: The function is used as I2C bus read
*	\Return : Status of the I2C read
*	\param dev_addr : The device address of the sensor
*	\param reg_addr : Address of the first register, will data is going to be read
*	\param reg_data : This data read from the sensor, which is hold in an array
*	\param cnt : The no of data byte of to be read
*/
int bh1750_I2C_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt) {
	int ret = 0;

	ESP_LOGD(TAG, "bh1750_I2C_write I2CAddress 0x%02X len %d reg 0x%02X", dev_addr,cnt,reg_addr);
	if (cnt>0 && reg_data != NULL && LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) {
		for (int pos = 0; pos < cnt; pos++) {
			printf("0x%02X ",*(reg_data + pos));
		}
		printf("\n");
	}
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, dev_addr<<1| I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
	if (cnt>0 && reg_data != NULL) {
		i2c_master_write(cmd, reg_data, cnt, ACK_CHECK_EN);
	}
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "bh1750_I2C_write write data fail I2CAddress 0x%02X len %d reg 0x%02X", dev_addr,cnt,reg_addr);
    }

	return ret;
}

 /*	\Brief: The function is used as I2C bus read
 *	\Return : Status of the I2C read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of data byte of to be read
 */
int bh1750_I2C_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt) {
	int ret = 0;
	int pos;
	i2c_cmd_handle_t cmd;

	ESP_LOGD(TAG, "bh1750_I2C_read I2CAddress 0x%02X len %d reg 0x%02X", dev_addr,cnt,reg_addr);

	if (reg_addr!=0xFF) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, dev_addr<<1| I2C_MASTER_WRITE, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
		i2c_master_stop(cmd);
		ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "bh1750_I2C_read write reg fail %d",ret);
			return ret;
		}
	}

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, dev_addr<<1| I2C_MASTER_READ, ACK_CHECK_EN);
	for (pos = 0; pos < (cnt-1); pos++) {
		i2c_master_read_byte(cmd, reg_data + pos, ACK_VAL);
	}
	i2c_master_read_byte(cmd, reg_data + cnt -1, NACK_VAL);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "bh1750_I2C_read read data fail %d",ret);
        return ret;
    }
    if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) {
		for (pos = 0; pos < cnt; pos++) {
			printf("0x%02X ",*(reg_data + pos));
		}
		printf("\n");
    }


	return ret;
}
