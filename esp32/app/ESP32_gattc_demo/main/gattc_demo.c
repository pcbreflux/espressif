// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
// Additions Copyright (C) Copyright 2016-2017 pcbreflux, Apache 2.0 License.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.



/****************************************************************************
*
* This file is for gatt client. It can scan ble device, connect one device,
*
****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "controller.h"

#include "bt.h"
#include "bt_trace.h"
#include "bt_types.h"
#include "btm_api.h"
#include "bta_api.h"
#include "bta_gatt_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_log.h"

#define GATTC_TAG "GATTC"

#define BT_BD_ADDR_STR         "%02x:%02x:%02x:%02x:%02x:%02x"
#define BT_BD_ADDR_HEX(addr)   addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

static esp_gatt_if_t client_if;
esp_gatt_status_t status = ESP_GATT_ERROR;
bool connet = false;
uint16_t simpleClient_id = 0xEE;

const char device_name[] = "nrf_si7021_gcc";

static esp_bd_addr_t server_dba;

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30
};


static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        //the unit of the duration is second
        uint32_t duration = 10;
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            ESP_LOGI(GATTC_TAG,  "BDA %x,%x,%x,%x,%x,%x:",scan_result->scan_rst.bda[0],
            		scan_result->scan_rst.bda[1],scan_result->scan_rst.bda[2],
					scan_result->scan_rst.bda[3],scan_result->scan_rst.bda[4],
					scan_result->scan_rst.bda[5]);
            for (int i = 0; i < 6; i++) {
                server_dba[i]=scan_result->scan_rst.bda[i];
            }
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            ESP_LOGI(GATTC_TAG,  "adv_name_len=%x\n", adv_name_len);
            for (int j = 0; j < adv_name_len; j++) {
                ESP_LOGI(GATTC_TAG,  "a%d %x %c = d%d %x %c",j, adv_name[j], adv_name[j],j, device_name[j], device_name[j]);
            }

            if (adv_name != NULL) {
                if (strncmp((char *)adv_name, device_name,adv_name_len) == 0) {
                    ESP_LOGI(GATTC_TAG,  "the name eque to %s.",device_name);
                    if (status ==  ESP_GATT_OK && connet == false) {
                        connet = true;
                        ESP_LOGI(GATTC_TAG,  "Connet to the remote device.");
                        esp_ble_gap_stop_scanning();
                        esp_ble_gattc_open(client_if, scan_result->scan_rst.bda, true);
                    }
                }
            }
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static uint16_t conn_id = 0;

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    ESP_LOGI(GATTC_TAG,  "esp_gattc_cb, event = %x", event);
    switch (event) {
    case ESP_GATTC_REG_EVT:
        status = p_data->reg.status;
        ESP_LOGI(GATTC_TAG,  "ESP_GATTC_REG_EVT status = %x, client_if = %x", status, gattc_if);
        break;
    case ESP_GATTC_OPEN_EVT:
        conn_id = p_data->open.conn_id;
        ESP_LOGI(GATTC_TAG,  "ESP_GATTC_OPEN_EVT conn_id %d, if %d, status %d", conn_id, gattc_if, p_data->open.status);
        esp_ble_gattc_search_service(gattc_if, conn_id, NULL);
        break;
    case ESP_GATTC_READ_CHAR_EVT: {
     	// esp_gatt_srvc_id_t *srvc_id = &p_data->read.srvc_id;
        //esp_gatt_id_t *char_id = &p_data->read.char_id;
        //conn_id = p_data->open.conn_id;
        //ESP_LOGI(GATTC_TAG,  "READ CHAR: open.conn_id = %x search_res.conn_id = %x  read.conn_id = %x", conn_id,p_data->search_res.conn_id,p_data->read.conn_id);
        ESP_LOGI(GATTC_TAG,  "READ CHAR: read.status = %x", p_data->read.status);
        if (p_data->read.status==0) {
           	esp_gattc_char_elem_t char_result;
            uint16_t char_count;
            esp_gatt_status_t ret_status = esp_ble_gattc_get_all_char(gattc_if,p_data->read.conn_id,p_data->read.handle,
            			p_data->read.handle,&char_result,&char_count,0);
			if (char_result.uuid.len == ESP_UUID_LEN_16) {
				ESP_LOGI(GATTC_TAG,  "Char UUID16: %x", char_result.uuid.uuid.uuid16);
			} else if (char_result.uuid.len == ESP_UUID_LEN_32) {
				ESP_LOGI(GATTC_TAG,  "Char UUID32: %x", char_result.uuid.uuid.uuid32);
			} else if (char_result.uuid.len == ESP_UUID_LEN_128) {
				ESP_LOGI(GATTC_TAG,  "Char UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", char_result.uuid.uuid.uuid128[0],
						 char_result.uuid.uuid.uuid128[1], char_result.uuid.uuid.uuid128[2], char_result.uuid.uuid.uuid128[3],
						 char_result.uuid.uuid.uuid128[4], char_result.uuid.uuid.uuid128[5], char_result.uuid.uuid.uuid128[6],
						 char_result.uuid.uuid.uuid128[7], char_result.uuid.uuid.uuid128[8], char_result.uuid.uuid.uuid128[9],
						 char_result.uuid.uuid.uuid128[10], char_result.uuid.uuid.uuid128[11], char_result.uuid.uuid.uuid128[12],
						 char_result.uuid.uuid.uuid128[13], char_result.uuid.uuid.uuid128[14], char_result.uuid.uuid.uuid128[15]);
			} else {
				ESP_LOGE(GATTC_TAG,  "Char UNKNOWN LEN %d\n", char_result.uuid.len);
			}
            for (int i = 0; i < p_data->read.value_len; i++) {
                ESP_LOGI(GATTC_TAG,  "%x:", p_data->read.value[i]);
            }
        }
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT: {
        // esp_gatt_srvc_id_t *srvc_id = &p_data->write.srvc_id;
        //esp_gatt_id_t *char_id = &p_data->write.char_id;
        //esp_gatt_id_t *descr_id = &p_data->write.descr_id;
        //conn_id = p_data->open.conn_id;
        //ESP_LOGI(GATTC_TAG,  "WRITE CHAR: open.conn_id = %x search_res.conn_id = %x  write.conn_id = %x", conn_id,p_data->search_res.conn_id,p_data->write.conn_id);
        ESP_LOGI(GATTC_TAG,  "WRITE CHAR: write.status = %x", p_data->write.status);
        if (p_data->write.status==0) {
           	esp_gattc_char_elem_t char_result;
            uint16_t char_count;
            esp_gatt_status_t ret_status;
            ret_status = esp_ble_gattc_get_all_char(gattc_if,p_data->write.conn_id,p_data->write.handle,
            			p_data->write.handle,&char_result,&char_count,0);
            esp_gattc_descr_elem_t decr_result;
            uint16_t decr_count;
            ret_status = esp_ble_gattc_get_all_descr(gattc_if,p_data->write.conn_id,p_data->write.handle,
                                                                 &decr_result, &decr_count,0);

			if (char_result.uuid.len == ESP_UUID_LEN_16) {
				ESP_LOGI(GATTC_TAG,  "Char UUID16: %x", char_result.uuid.uuid.uuid16);
			} else if (char_result.uuid.len == ESP_UUID_LEN_32) {
				ESP_LOGI(GATTC_TAG,  "Char UUID32: %x", char_result.uuid.uuid.uuid32);
			} else if (char_result.uuid.len == ESP_UUID_LEN_128) {
				ESP_LOGI(GATTC_TAG,  "Char UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", char_result.uuid.uuid.uuid128[0],
						 char_result.uuid.uuid.uuid128[1], char_result.uuid.uuid.uuid128[2], char_result.uuid.uuid.uuid128[3],
						 char_result.uuid.uuid.uuid128[4], char_result.uuid.uuid.uuid128[5], char_result.uuid.uuid.uuid128[6],
						 char_result.uuid.uuid.uuid128[7], char_result.uuid.uuid.uuid128[8], char_result.uuid.uuid.uuid128[9],
						 char_result.uuid.uuid.uuid128[10], char_result.uuid.uuid.uuid128[11], char_result.uuid.uuid.uuid128[12],
						 char_result.uuid.uuid.uuid128[13], char_result.uuid.uuid.uuid128[14], char_result.uuid.uuid.uuid128[15]);
			} else {
				ESP_LOGE(GATTC_TAG,  "Char UNKNOWN LEN %d", char_result.uuid.len);
			}
			if (decr_result.uuid.len == ESP_UUID_LEN_16) {
				ESP_LOGI(GATTC_TAG,  "Decr UUID16: %x", decr_result.uuid.uuid.uuid16);
			} else if (decr_result.uuid.len == ESP_UUID_LEN_32) {
				ESP_LOGI(GATTC_TAG,  "Decr UUID32: %x", decr_result.uuid.uuid.uuid32);
			} else if (decr_result.uuid.len == ESP_UUID_LEN_128) {
				ESP_LOGI(GATTC_TAG,  "Decr UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", decr_result.uuid.uuid.uuid128[0],
						 decr_result.uuid.uuid.uuid128[1], decr_result.uuid.uuid.uuid128[2], decr_result.uuid.uuid.uuid128[3],
						 decr_result.uuid.uuid.uuid128[4], decr_result.uuid.uuid.uuid128[5], decr_result.uuid.uuid.uuid128[6],
						 decr_result.uuid.uuid.uuid128[7], decr_result.uuid.uuid.uuid128[8], decr_result.uuid.uuid.uuid128[9],
						 decr_result.uuid.uuid.uuid128[10], decr_result.uuid.uuid.uuid128[11], decr_result.uuid.uuid.uuid128[12],
						 decr_result.uuid.uuid.uuid128[13], decr_result.uuid.uuid.uuid128[14], decr_result.uuid.uuid.uuid128[15]);
			} else {
				ESP_LOGE(GATTC_TAG,  "Decr UNKNOWN LEN %d", decr_result.uuid.len);
			}
        }
        break;
    }
    case ESP_GATTC_SEARCH_RES_EVT: {
        esp_gatt_srvc_id_t *srvc_id = (esp_gatt_srvc_id_t *)&p_data->search_res.srvc_id;
        //conn_id = p_data->open.conn_id;
        ESP_LOGI(GATTC_TAG,  "SEARCH RES: open.conn_id = %x search_res.conn_id = %x", conn_id,p_data->search_res.conn_id);
        if (srvc_id->id.uuid.len == ESP_UUID_LEN_16) {
            ESP_LOGI(GATTC_TAG,  "UUID16: %x", srvc_id->id.uuid.uuid.uuid16);
        } else if (srvc_id->id.uuid.len == ESP_UUID_LEN_32) {
            ESP_LOGI(GATTC_TAG,  "UUID32: %x", srvc_id->id.uuid.uuid.uuid32);
        } else if (srvc_id->id.uuid.len == ESP_UUID_LEN_128) {
            ESP_LOGI(GATTC_TAG,  "UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", srvc_id->id.uuid.uuid.uuid128[0],
                     srvc_id->id.uuid.uuid.uuid128[1], srvc_id->id.uuid.uuid.uuid128[2], srvc_id->id.uuid.uuid.uuid128[3],
                     srvc_id->id.uuid.uuid.uuid128[4], srvc_id->id.uuid.uuid.uuid128[5], srvc_id->id.uuid.uuid.uuid128[6],
                     srvc_id->id.uuid.uuid.uuid128[7], srvc_id->id.uuid.uuid.uuid128[8], srvc_id->id.uuid.uuid.uuid128[9],
                     srvc_id->id.uuid.uuid.uuid128[10], srvc_id->id.uuid.uuid.uuid128[11], srvc_id->id.uuid.uuid.uuid128[12],
                     srvc_id->id.uuid.uuid.uuid128[13], srvc_id->id.uuid.uuid.uuid128[14], srvc_id->id.uuid.uuid.uuid128[15]);
        } else {
            ESP_LOGE(GATTC_TAG,  "UNKNOWN LEN %d", srvc_id->id.uuid.len);
        }
        break;
    }
    case ESP_GATTC_WRITE_DESCR_EVT: {
        //esp_gatt_srvc_id_t *srvc_id = &p_data->write.srvc_id;
        //esp_gatt_id_t *char_id = &p_data->write.char_id;
        //esp_gatt_id_t *descr_id = &p_data->write.descr_id;
        //conn_id = p_data->open.conn_id;
        //ESP_LOGI(GATTC_TAG,  "WRITE DESCR: open.conn_id = %x search_res.conn_id = %x  write.conn_id = %x", conn_id,p_data->search_res.conn_id,p_data->write.conn_id);
        ESP_LOGI(GATTC_TAG,  "WRITE DESCR: write.status = %x open.gatt_if = %x", p_data->write.status,gattc_if);
        if (p_data->write.status==0) {
           	esp_gattc_char_elem_t char_result;
            uint16_t char_count;
            esp_gatt_status_t ret_status;
            ret_status = esp_ble_gattc_get_all_char(gattc_if,p_data->write.conn_id,p_data->write.handle,
            			p_data->write.handle,&char_result,&char_count,0);
            esp_gattc_descr_elem_t decr_result;
            uint16_t decr_count;
            ret_status = esp_ble_gattc_get_all_descr(gattc_if,p_data->write.conn_id,p_data->write.handle,
                                                                 &decr_result, &decr_count,0);

			if (char_result.uuid.len == ESP_UUID_LEN_16) {
				ESP_LOGI(GATTC_TAG,  "Char UUID16: %x", char_result.uuid.uuid.uuid16);
			} else if (char_result.uuid.len == ESP_UUID_LEN_32) {
				ESP_LOGI(GATTC_TAG,  "Char UUID32: %x", char_result.uuid.uuid.uuid32);
			} else if (char_result.uuid.len == ESP_UUID_LEN_128) {
				ESP_LOGI(GATTC_TAG,  "Char UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", char_result.uuid.uuid.uuid128[0],
						 char_result.uuid.uuid.uuid128[1], char_result.uuid.uuid.uuid128[2], char_result.uuid.uuid.uuid128[3],
						 char_result.uuid.uuid.uuid128[4], char_result.uuid.uuid.uuid128[5], char_result.uuid.uuid.uuid128[6],
						 char_result.uuid.uuid.uuid128[7], char_result.uuid.uuid.uuid128[8], char_result.uuid.uuid.uuid128[9],
						 char_result.uuid.uuid.uuid128[10], char_result.uuid.uuid.uuid128[11], char_result.uuid.uuid.uuid128[12],
						 char_result.uuid.uuid.uuid128[13], char_result.uuid.uuid.uuid128[14], char_result.uuid.uuid.uuid128[15]);
			} else {
				ESP_LOGE(GATTC_TAG,  "Char UNKNOWN LEN %d", char_result.uuid.len);
			}
			if (decr_result.uuid.len == ESP_UUID_LEN_16) {
				ESP_LOGI(GATTC_TAG,  "Decr UUID16: %x", decr_result.uuid.uuid.uuid16);
			} else if (decr_result.uuid.len == ESP_UUID_LEN_32) {
				ESP_LOGI(GATTC_TAG,  "Decr UUID32: %x", decr_result.uuid.uuid.uuid32);
			} else if (decr_result.uuid.len == ESP_UUID_LEN_128) {
				ESP_LOGI(GATTC_TAG,  "Decr UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", decr_result.uuid.uuid.uuid128[0],
						 decr_result.uuid.uuid.uuid128[1], decr_result.uuid.uuid.uuid128[2], decr_result.uuid.uuid.uuid128[3],
						 decr_result.uuid.uuid.uuid128[4], decr_result.uuid.uuid.uuid128[5], decr_result.uuid.uuid.uuid128[6],
						 decr_result.uuid.uuid.uuid128[7], decr_result.uuid.uuid.uuid128[8], decr_result.uuid.uuid.uuid128[9],
						 decr_result.uuid.uuid.uuid128[10], decr_result.uuid.uuid.uuid128[11], decr_result.uuid.uuid.uuid128[12],
						 decr_result.uuid.uuid.uuid128[13], decr_result.uuid.uuid.uuid128[14], decr_result.uuid.uuid.uuid128[15]);
			} else {
				ESP_LOGE(GATTC_TAG,  "Decr UNKNOWN LEN %d", decr_result.uuid.len);
			}
	        ESP_LOGI(GATTC_TAG,  "WRITE DESCR: gattc_if = %x",gattc_if);
            ESP_LOGI(GATTC_TAG,  "remote_bda %x,%x,%x,%x,%x,%x:",p_data->open.remote_bda[0],
            		p_data->open.remote_bda[1],p_data->open.remote_bda[2],
					p_data->open.remote_bda[3],p_data->open.remote_bda[4],
					p_data->open.remote_bda[5]);
            ESP_LOGI(GATTC_TAG,  "server_dba %x,%x,%x,%x,%x,%x:",server_dba[0],
            		server_dba[1],server_dba[2],
					server_dba[3],server_dba[4],
					server_dba[5]);
			esp_ble_gattc_register_for_notify(gattc_if,server_dba,p_data->write.handle);
        }
        break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
        // esp_gatt_srvc_id_t *srvc_id = &p_data->read.srvc_id;
        //esp_gatt_id_t *char_id = &p_data->notify.char_id;
        //conn_id = p_data->open.conn_id;
        //ESP_LOGI(GATTC_TAG,  "NOTIFY: open.conn_id = %x search_res.conn_id = %x  notify.conn_id = %x", conn_id,p_data->search_res.conn_id,p_data->notify.conn_id);
        ESP_LOGI(GATTC_TAG,  "NOTIFY: notify.is_notify = %x", p_data->notify.is_notify);
        if (p_data->notify.is_notify==1) {
           	esp_gattc_char_elem_t char_result;
            uint16_t char_count;
            esp_gatt_status_t ret_status;
            ret_status = esp_ble_gattc_get_all_char(gattc_if,p_data->notify.conn_id,p_data->notify.handle,
            			p_data->notify.handle,&char_result,&char_count,0);
			if (char_result.uuid.len == ESP_UUID_LEN_16) {
				ESP_LOGI(GATTC_TAG,  "Char UUID16: %x", char_result.uuid.uuid.uuid16);
			} else if (char_result.uuid.len == ESP_UUID_LEN_32) {
				ESP_LOGI(GATTC_TAG,  "Char UUID32: %x", char_result.uuid.uuid.uuid32);
			} else if (char_result.uuid.len == ESP_UUID_LEN_128) {
				ESP_LOGI(GATTC_TAG,  "Char UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", char_result.uuid.uuid.uuid128[0],
						 char_result.uuid.uuid.uuid128[1], char_result.uuid.uuid.uuid128[2], char_result.uuid.uuid.uuid128[3],
						 char_result.uuid.uuid.uuid128[4], char_result.uuid.uuid.uuid128[5], char_result.uuid.uuid.uuid128[6],
						 char_result.uuid.uuid.uuid128[7], char_result.uuid.uuid.uuid128[8], char_result.uuid.uuid.uuid128[9],
						 char_result.uuid.uuid.uuid128[10], char_result.uuid.uuid.uuid128[11], char_result.uuid.uuid.uuid128[12],
						 char_result.uuid.uuid.uuid128[13], char_result.uuid.uuid.uuid128[14], char_result.uuid.uuid.uuid128[15]);
			} else {
				ESP_LOGE(GATTC_TAG,  "Char UNKNOWN LEN %d\n", char_result.uuid.len);
			}
            for (int i = 0; i < p_data->notify.value_len; i++) {
                ESP_LOGI(GATTC_TAG,  "NOTIFY: V%d %x:", i, p_data->notify.value[i]);
            }
        }
        break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        ESP_LOGI(GATTC_TAG,  "NOTIFY_EVT: open.conn_id = %x ", conn_id);
        ESP_LOGI(GATTC_TAG,  "NOTIFY_EVT: reg_for_notify.status = %x ", p_data->reg_for_notify.status);
        //esp_gatt_id_t *char_id = &p_data->reg_for_notify.char_id;
        if (p_data->reg_for_notify.status==0) {
           	esp_gattc_char_elem_t char_result;
            uint16_t char_count;
            esp_gatt_status_t ret_status;
            ret_status = esp_ble_gattc_get_all_char(gattc_if,conn_id,p_data->reg_for_notify.handle,
            			p_data->reg_for_notify.handle,&char_result,&char_count,0);
			if (char_result.uuid.len == ESP_UUID_LEN_16) {
				ESP_LOGI(GATTC_TAG,  "UUID16: %x", char_result.uuid.uuid.uuid16);
			} else if (char_result.uuid.len == ESP_UUID_LEN_32) {
				ESP_LOGI(GATTC_TAG,  "UUID32: %x", char_result.uuid.uuid.uuid32);
			} else if (char_result.uuid.len == ESP_UUID_LEN_128) {
				ESP_LOGI(GATTC_TAG,  "UUID128: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", char_result.uuid.uuid.uuid128[0],
						 char_result.uuid.uuid.uuid128[1], char_result.uuid.uuid.uuid128[2], char_result.uuid.uuid.uuid128[3],
						 char_result.uuid.uuid.uuid128[4], char_result.uuid.uuid.uuid128[5], char_result.uuid.uuid.uuid128[6],
						 char_result.uuid.uuid.uuid128[7], char_result.uuid.uuid.uuid128[8], char_result.uuid.uuid.uuid128[9],
						 char_result.uuid.uuid.uuid128[10], char_result.uuid.uuid.uuid128[11], char_result.uuid.uuid.uuid128[12],
						 char_result.uuid.uuid.uuid128[13], char_result.uuid.uuid.uuid128[14], char_result.uuid.uuid.uuid128[15]);
			} else {
				ESP_LOGE(GATTC_TAG,  "UNKNOWN LEN %d", char_result.uuid.len);
			}
        }
        break;
        uint8_t value[2];
        value[0]=0x01;
        value[1]=0x00;
		esp_ble_gattc_write_char_descr(gattc_if,conn_id,p_data->reg_for_notify.handle,2,&value[0],ESP_GATT_WRITE_TYPE_NO_RSP,ESP_GATT_AUTH_REQ_NONE);
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        //conn_id = p_data->search_cmpl.conn_id;
        ESP_LOGI(GATTC_TAG,  "SEARCH_CMPL: conn_id = %x, status %d", conn_id, p_data->search_cmpl.status);
        break;
    default:
        break;
    }
}

void ble_client_appRegister(void)
{
    ESP_LOGI(GATTC_TAG,  "register callback");

    //register the scan callback function to the Generic Access Profile (GAP) module
    if ((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK) {
        ESP_LOGE(GATTC_TAG,  "gap register error, error code = %x", status);
        return;
    }

    //register the callback function to the Generic Attribute Profile (GATT) Client (GATTC) module
    if ((status = esp_ble_gattc_register_callback(esp_gattc_cb)) != ESP_OK) {
        ESP_LOGE(GATTC_TAG,  "gattc register error, error code = %x", status);
        return;
    }
    esp_ble_gattc_app_register(simpleClient_id);
    esp_ble_gap_set_scan_params(&ble_scan_params);
}

void gattc_client_test(void)
{
    esp_bluedroid_init();
    esp_bluedroid_enable();
    ble_client_appRegister();
}

void app_main()
{
    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTC_TAG,  "%s initialize controller failed\n", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    if (ret) {
        ESP_LOGE(GATTC_TAG,  "%s enable controller failed\n", __func__);
        return;
    }

    gattc_client_test();
}

