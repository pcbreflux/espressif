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
#include <stdlib.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "bt.h"
#include "bta_api.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_main.h"

#include <driver/rmt.h>
#include <driver/gpio.h>

#include "sdkconfig.h"

#include "ble_uart_server.h"

#define GATTS_TAG "GATTS"

#define LED_PIN GPIO_NUM_22
#define HIGH 1
#define LOW 0

static uint8_t led_stat=0;


uint8_t char1_str[GATTS_CHAR_VAL_LEN_MAX] = {0x11,0x22,0x33};
uint8_t char2_str[GATTS_CHAR_VAL_LEN_MAX] = {0x11,0x22,0x33};
uint8_t descr1_str[GATTS_CHAR_VAL_LEN_MAX] = {0x00,0x00};
uint8_t descr2_str[GATTS_CHAR_VAL_LEN_MAX] = "Hallo ESP32";

esp_attr_value_t gatts_demo_char1_val = {
	.attr_max_len = GATTS_CHAR_VAL_LEN_MAX,
	.attr_len		= sizeof(char1_str),
	.attr_value     = char1_str,
};

esp_attr_value_t gatts_demo_char2_val = {
	.attr_max_len = GATTS_CHAR_VAL_LEN_MAX,
	.attr_len		= sizeof(char2_str),
	.attr_value     = char2_str,
};

esp_attr_value_t gatts_demo_descr1_val = {
	.attr_max_len = GATTS_CHAR_VAL_LEN_MAX,
	.attr_len		= sizeof(descr1_str),
	.attr_value     = descr1_str,
};

esp_attr_value_t gatts_demo_descr2_val = {
	.attr_max_len = GATTS_CHAR_VAL_LEN_MAX,
	.attr_len		= 11,
	.attr_value     = descr2_str,
};

#define BLE_SERVICE_UUID_SIZE ESP_UUID_LEN_128

// Add more UUIDs for more then one Service
static uint8_t ble_service_uuid128[BLE_SERVICE_UUID_SIZE] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
	 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E,
    // 0xXfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAB, 0xCD, 0x00, 0x00,
    //0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static uint8_t ble_manufacturer[BLE_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};

static uint32_t ble_add_char_pos;

static esp_ble_adv_data_t ble_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = BLE_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  (uint8_t *)ble_manufacturer,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = BLE_SERVICE_UUID_SIZE,
    .p_service_uuid = ble_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t ble_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

struct gatts_char_inst {
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t char_perm;
	esp_gatt_char_prop_t char_property;
	esp_attr_value_t *char_val;
    esp_attr_control_t *char_control;
    uint16_t char_handle;
    esp_gatts_cb_t char_read_callback;
	esp_gatts_cb_t char_write_callback;
    esp_bt_uuid_t descr_uuid;
    esp_gatt_perm_t descr_perm;
	esp_attr_value_t *descr_val;
    esp_attr_control_t *descr_control;
    uint16_t descr_handle;
    esp_gatts_cb_t descr_read_callback;
	esp_gatts_cb_t descr_write_callback;
};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst gl_profile = {
         .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
};


/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_char_inst gl_char[GATTS_CHAR_NUM] = {
		{
				.char_uuid.len = ESP_UUID_LEN_128, // RX
				.char_uuid.uuid.uuid128 =  { 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E },
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
				.char_val = &gatts_demo_char1_val,
				.char_control = NULL,
				.char_handle = 0,
				.char_read_callback=char1_read_handler,
				.char_write_callback=char1_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr1_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr1_read_handler,
				.descr_write_callback=descr1_write_handler
		},
		{
				.char_uuid.len = ESP_UUID_LEN_128,  // TX
				.char_uuid.uuid.uuid128 =  { 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E },
				.char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
				.char_val = &gatts_demo_char2_val,
				.char_control=NULL,
				.char_handle=0,
				.char_read_callback=char2_read_handler,
				.char_write_callback=char2_write_handler,
				.descr_uuid.len = ESP_UUID_LEN_16,
				.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG, // ESP_GATT_UUID_CHAR_DESCRIPTION,
				.descr_perm=ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
				.descr_val = &gatts_demo_descr2_val,
				.descr_control=NULL,
				.descr_handle=0,
				.descr_read_callback=descr2_read_handler,
				.descr_write_callback=descr2_write_handler
		}
};

void char1_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	ESP_LOGI(GATTS_TAG, "char1_read_handler %d\n", param->read.handle);

	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[0].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char1_read_handler char_val %d\n",gl_char[0].char_val->attr_len);
		rsp.attr_value.len = gl_char[0].char_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[0].char_val->attr_len&&pos<gl_char[0].char_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[0].char_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char1_read_handler esp_gatt_rsp_t\n");
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
								ESP_GATT_OK, &rsp);
}

void char2_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	ESP_LOGI(GATTS_TAG, "char2_read_handler %d\n", param->read.handle);

	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[1].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char2_read_handler char_val\n %d\n",gl_char[1].char_val->attr_len);
		rsp.attr_value.len = gl_char[1].char_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[1].char_val->attr_len&&pos<gl_char[1].char_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[1].char_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char2_read_handler esp_gatt_rsp_t\n");
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
								ESP_GATT_OK, &rsp);
}

void descr1_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	ESP_LOGI(GATTS_TAG, "descr1_read_handler %d\n", param->read.handle);

	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[0].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr1_read_handler descr_val %d\n",gl_char[0].descr_val->attr_len);
		rsp.attr_value.len = gl_char[0].descr_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[0].descr_val->attr_len&&pos<gl_char[0].descr_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[0].descr_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr1_read_handler esp_gatt_rsp_t \n");
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
								ESP_GATT_OK, &rsp);
}

void descr2_read_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	ESP_LOGI(GATTS_TAG, "descr2_read_handler %d\n", param->read.handle);

	esp_gatt_rsp_t rsp;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	if (gl_char[1].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr2_read_handler descr_val %d\n",gl_char[1].descr_val->attr_len);
		rsp.attr_value.len = gl_char[1].descr_val->attr_len;
		for (uint32_t pos=0;pos<gl_char[1].descr_val->attr_len&&pos<gl_char[1].descr_val->attr_max_len;pos++) {
			rsp.attr_value.value[pos] = gl_char[1].descr_val->attr_value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr2_read_handler esp_gatt_rsp_t\n");
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
								ESP_GATT_OK, &rsp);
}

static  uint16_t notify_conn_id = 0;
static  esp_gatt_if_t notify_gatts_if = NULL;
static uint8_t notify_pos=0;
static uint8_t is_notify=0;


void char2_notify_handle(esp_gatt_if_t gatts_if, uint16_t conn_id) {
	if (is_notify==1) {
		notify_pos='0';
		for (uint32_t i=0;i<10;i++) {
			ESP_LOGI(GATTS_TAG, "char2_notify_handle esp_ble_gatts_send_indicate\n");
			// vTaskDelay(1000 / portTICK_RATE_MS); // delay 1s
			esp_ble_gatts_send_indicate(gatts_if, conn_id, gl_char[1].char_handle,1,&notify_pos,false);
			notify_pos++;
		}
	}
}

void char1_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	ESP_LOGI(GATTS_TAG, "char1_write_handler %d\n", param->write.handle);

	if (gl_char[0].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char1_write_handler char_val %d\n",param->write.len);
		gl_char[0].char_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[0].char_val->attr_value[pos]=param->write.value[pos];
		}
		ESP_LOGI(TAG, "char1_write_handler %.*s", gl_char[0].char_val->attr_len, (char*)gl_char[0].char_val->attr_value);
	}
	ESP_LOGI(GATTS_TAG, "char1_write_handler esp_gatt_rsp_t\n");
	notify_gatts_if = gatts_if;
	notify_conn_id = param->write.conn_id;
    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
    if (strncmp((const char *)gl_char[0].char_val->attr_value,"LED ON",6)==0) {
    	gpio_set_level(LED_PIN,HIGH);
    	led_stat=1;
    } else if (strncmp((const char *)gl_char[0].char_val->attr_value,"LED OFF",7)==0) {
    	gpio_set_level(LED_PIN,LOW);
    	led_stat=0;
    } else if (strncmp((const char *)gl_char[0].char_val->attr_value,"LED SWITCH",10)==0) {
    	led_stat=1-led_stat;
    	gpio_set_level(LED_PIN,led_stat);
    } else {
    	char2_notify_handle(gatts_if, param->write.conn_id);
    }
}

void char2_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	ESP_LOGI(GATTS_TAG, "char2_write_handler %d\n", param->write.handle);

	if (gl_char[1].char_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "char2_write_handler char_val %d\n",param->write.len);
		gl_char[1].char_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[1].char_val->attr_value[pos]=param->write.value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "char2_write_handler esp_gatt_rsp_t\n");
    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}

void descr1_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	ESP_LOGI(GATTS_TAG, "descr1_write_handler %d\n", param->write.handle);

	if (gl_char[0].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr1_write_handler descr_val %d\n",param->write.len);
		gl_char[0].descr_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[0].descr_val->attr_value[pos]=param->write.value[pos];
		}
	}
	ESP_LOGI(GATTS_TAG, "descr1_write_handler esp_gatt_rsp_t\n");
    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}

void descr2_write_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	ESP_LOGI(GATTS_TAG, "descr2_write_handler %d\n", param->write.handle);

	if (gl_char[1].descr_val!=NULL) {
		ESP_LOGI(GATTS_TAG, "descr2_write_handler descr_val %d\n",param->write.len);
		gl_char[1].descr_val->attr_len = param->write.len;
		for (uint32_t pos=0;pos<param->write.len;pos++) {
			gl_char[1].descr_val->attr_value[pos]=param->write.value[pos];
		}
		is_notify = gl_char[1].descr_val->attr_value[0];
		ESP_LOGI(GATTS_TAG, "descr1_write_handler is_notify %d\n",is_notify);
	}
	ESP_LOGI(GATTS_TAG, "descr2_write_handler esp_gatt_rsp_t\n");
    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}

void gatts_add_char() {

	ESP_LOGI(GATTS_TAG, "gatts_add_char %d\n", GATTS_CHAR_NUM);
	for (uint32_t pos=0;pos<GATTS_CHAR_NUM;pos++) {
		if (gl_char[pos].char_handle==0) {
			ESP_LOGI(GATTS_TAG, "ADD pos %d handle %d service %d\n", pos,gl_char[pos].char_handle,gl_profile.service_handle);
			ble_add_char_pos=pos;
			esp_ble_gatts_add_char(gl_profile.service_handle, &gl_char[pos].char_uuid,
								   gl_char[pos].char_perm,gl_char[pos].char_property,gl_char[pos].char_val, gl_char[pos].char_control);
			break;
		}
	}
}

void gatts_check_add_char(esp_bt_uuid_t char_uuid, uint16_t attr_handle) {

	ESP_LOGI(GATTS_TAG, "gatts_check_add_char %d\n", attr_handle);
	if (attr_handle != 0) {
		if (char_uuid.len == ESP_UUID_LEN_16) {
			ESP_LOGI(GATTS_TAG, "Char UUID16: %x", char_uuid.uuid.uuid16);
		} else if (char_uuid.len == ESP_UUID_LEN_32) {
			ESP_LOGI(GATTS_TAG, "Char UUID32: %x", char_uuid.uuid.uuid32);
		} else if (char_uuid.len == ESP_UUID_LEN_128) {
			ESP_LOGI(GATTS_TAG, "Char UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", char_uuid.uuid.uuid128[0],
					 char_uuid.uuid.uuid128[1], char_uuid.uuid.uuid128[2], char_uuid.uuid.uuid128[3],
					 char_uuid.uuid.uuid128[4], char_uuid.uuid.uuid128[5], char_uuid.uuid.uuid128[6],
					 char_uuid.uuid.uuid128[7], char_uuid.uuid.uuid128[8], char_uuid.uuid.uuid128[9],
					 char_uuid.uuid.uuid128[10], char_uuid.uuid.uuid128[11], char_uuid.uuid.uuid128[12],
					 char_uuid.uuid.uuid128[13], char_uuid.uuid.uuid128[14], char_uuid.uuid.uuid128[15]);
		} else {
			ESP_LOGE(GATTS_TAG, "Char UNKNOWN LEN %d\n", char_uuid.len);
		}

		ESP_LOGI(GATTS_TAG, "FOUND Char pos %d handle %d\n", ble_add_char_pos,attr_handle);
		gl_char[ble_add_char_pos].char_handle=attr_handle;

		// is there a descriptor to add ?
		if (gl_char[ble_add_char_pos].descr_uuid.len!=0 && gl_char[ble_add_char_pos].descr_handle==0) {
			ESP_LOGI(GATTS_TAG, "ADD Descr pos %d handle %d service %d\n", ble_add_char_pos,gl_char[ble_add_char_pos].descr_handle,gl_profile.service_handle);
			esp_ble_gatts_add_char_descr(gl_profile.service_handle, &gl_char[ble_add_char_pos].descr_uuid,
					gl_char[ble_add_char_pos].descr_perm, gl_char[ble_add_char_pos].descr_val, gl_char[ble_add_char_pos].descr_control);
		} else {
			gatts_add_char();
		}
	}
}



void gatts_check_add_descr(esp_bt_uuid_t descr_uuid, uint16_t attr_handle) {

	ESP_LOGI(GATTS_TAG, "gatts_check_add_descr %d\n", attr_handle);
	if (attr_handle != 0) {
		if (descr_uuid.len == ESP_UUID_LEN_16) {
			ESP_LOGI(GATTS_TAG, "Char UUID16: %x", descr_uuid.uuid.uuid16);
		} else if (descr_uuid.len == ESP_UUID_LEN_32) {
			ESP_LOGI(GATTS_TAG, "Char UUID32: %x", descr_uuid.uuid.uuid32);
		} else if (descr_uuid.len == ESP_UUID_LEN_128) {
			ESP_LOGI(GATTS_TAG, "Char UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", descr_uuid.uuid.uuid128[0],
					 descr_uuid.uuid.uuid128[1], descr_uuid.uuid.uuid128[2], descr_uuid.uuid.uuid128[3],
					 descr_uuid.uuid.uuid128[4], descr_uuid.uuid.uuid128[5], descr_uuid.uuid.uuid128[6],
					 descr_uuid.uuid.uuid128[7], descr_uuid.uuid.uuid128[8], descr_uuid.uuid.uuid128[9],
					 descr_uuid.uuid.uuid128[10], descr_uuid.uuid.uuid128[11], descr_uuid.uuid.uuid128[12],
					 descr_uuid.uuid.uuid128[13], descr_uuid.uuid.uuid128[14], descr_uuid.uuid.uuid128[15]);
		} else {
			ESP_LOGE(GATTS_TAG, "Descriptor UNKNOWN LEN %d\n", descr_uuid.len);
		}
		ESP_LOGI(GATTS_TAG, "FOUND Descriptor pos %d handle %d\n", ble_add_char_pos,attr_handle);
		gl_char[ble_add_char_pos].descr_handle=attr_handle;
	}
	gatts_add_char();
}

void gatts_check_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	uint16_t handle=0;
	uint8_t read=1;

    switch (event) {
		case ESP_GATTS_READ_EVT: {
			read=1;
			handle=param->read.handle;
			break;
		}
		case ESP_GATTS_WRITE_EVT: {
			read=0;
			handle=param->write.handle;
		}
		default:
			break;
    }

	ESP_LOGI(GATTS_TAG, "gatts_check_callback read %d num %d handle %d\n", read, GATTS_CHAR_NUM, handle);
	for (uint32_t pos=0;pos<GATTS_CHAR_NUM;pos++) {
		if (gl_char[pos].char_handle==handle) {
			if (read==1) {
				if (gl_char[pos].char_read_callback!=NULL) {
					gl_char[pos].char_read_callback(event, gatts_if, param);
				}
			} else {
				if (gl_char[pos].char_write_callback!=NULL) {
					gl_char[pos].char_write_callback(event, gatts_if, param);
				}
			}
			break;
		}
		if (gl_char[pos].descr_handle==handle) {
			if (read==1) {
				if (gl_char[pos].descr_read_callback!=NULL) {
					gl_char[pos].descr_read_callback(event, gatts_if, param);
				}
			} else {
				if (gl_char[pos].descr_write_callback!=NULL) {
					gl_char[pos].descr_write_callback(event, gatts_if, param);
				}
			}
			break;
		}
	}
}


void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&ble_adv_params);
        break;
    default:
        break;
    }
}

void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	esp_err_t ret;
	switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        gl_profile.service_id.is_primary = true;
        gl_profile.service_id.id.inst_id = 0x00;
        gl_profile.service_id.id.uuid.len = ESP_UUID_LEN_128;
        for (uint8_t pos=0;pos<ESP_UUID_LEN_128;pos++) {
        	gl_profile.service_id.id.uuid.uuid.uuid128[pos]=ble_service_uuid128[pos];
        }

        esp_ble_gap_set_device_name(BLE_DEVICE_NAME);
        ret=esp_ble_gap_config_adv_data(&ble_adv_data);
        ESP_LOGI(GATTS_TAG, "esp_ble_gap_config_adv_data %d", ret);

        esp_ble_gatts_create_service(gatts_if, &gl_profile.service_id, GATTS_NUM_HANDLE);
        break;
    case ESP_GATTS_READ_EVT: {
        ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
//        esp_gatt_rsp_t rsp;
//        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
//        rsp.attr_value.handle = param->read.handle;
//        rsp.attr_value.len = 4;
//        rsp.attr_value.value[0] = 0x0A;
//        rsp.attr_value.value[1] = 0x0B;
//        rsp.attr_value.value[2] = 0x0C;
//        rsp.attr_value.value[3] = 0x0D;
//        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
//                                    ESP_GATT_OK, &rsp);
        gatts_check_callback(event, gatts_if, param);
        break;
    }
    case ESP_GATTS_WRITE_EVT: {
        ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d\n", param->write.conn_id, param->write.trans_id, param->write.handle);
        ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value %08x\n", param->write.len, *(uint32_t *)param->write.value);
//       esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        gatts_check_callback(event, gatts_if, param);
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:
    case ESP_GATTS_MTU_EVT:
    case ESP_GATTS_CONF_EVT:
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile.service_handle = param->create.service_handle;
        gl_profile.char_uuid.len = gl_char[0].char_uuid.len;
        gl_profile.char_uuid.uuid.uuid16 = gl_char[0].char_uuid.uuid.uuid16;

        esp_ble_gatts_start_service(gl_profile.service_handle);
        gatts_add_char();
//        esp_ble_gatts_add_char(gl_profile.service_handle, &gl_profile.char_uuid,
//                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
//                               ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
//                               &gatts_demo_char1_val, NULL);
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT: {
//	    uint16_t length = 0;
//        const uint8_t *prf_char;

        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status 0x%X,  attr_handle %d, service_handle %d\n",
                param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        gl_profile.char_handle = param->add_char.attr_handle;
//        gl_profile.descr_uuid.len = ESP_UUID_LEN_16;
//        gl_profile.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
//        esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);

//        ESP_LOGI(GATTS_TAG, "the gatts demo char length = %x\n", length);
//        for(int i = 0; i < length; i++){
//            ESP_LOGI(GATTS_TAG, "prf_char[%x] =%x\n",i,prf_char[i]);
//        }
//        esp_ble_gatts_add_char_descr(gl_profile.service_handle, &gl_profile.descr_uuid,
//                                     ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
        if (param->add_char.status==ESP_GATT_OK) {
        	gatts_check_add_char(param->add_char.char_uuid,param->add_char.attr_handle);
        }
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT char, status %d, attr_handle %d, service_handle %d\n",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT desc, status %d, attr_handle %d, service_handle %d\n",
                 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        if (param->add_char_descr.status==ESP_GATT_OK) {
        	gatts_check_add_descr(param->add_char.char_uuid,param->add_char.attr_handle);
        }
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x: \n",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile.conn_id = param->connect.conn_id;
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        esp_ble_gap_start_advertising(&ble_adv_params);
        break;
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
        	gl_profile.gatts_if = gatts_if;

        	uint64_t bitmask = 0;
        	bitmask = bitmask | (1<<LED_PIN);

        	gpio_config_t gpioConfig;
        	gpioConfig.pin_bit_mask = bitmask;
        	gpioConfig.mode         = GPIO_MODE_OUTPUT;
        	gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
        	gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
        	gpioConfig.intr_type    = GPIO_INTR_DISABLE;
        	gpio_config(&gpioConfig);

        	gpio_set_level(LED_PIN, LOW);
        } else {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id, 
                    param->reg.status);
            return;
        }
    }

    gatts_profile_event_handler(event, gatts_if, param);
}
