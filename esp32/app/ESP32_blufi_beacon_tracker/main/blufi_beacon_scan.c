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
#include <math.h>
#include "bt.h"

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_blufi_api.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "blufi_beacon_scan.h"
#include "blufi_https_request.h"

#define GAP_TAG "BEACON_DEMO"

#define BLUFI_BEACON_START_LEN 9

static uint8_t blufi_beacon_start[BLUFI_BEACON_START_LEN] = {
     0x02,0x01,0x04,0x1A,0xFF,0x4C,0x00,0x02,0x15
};


static esp_bd_addr_t server_dba;
static uint32_t gapcount = 0;
#define BLUFI_MAX_BEACON 30
static blufi_beacon_t blufi_beacons[BLUFI_MAX_BEACON];

double getDistance(int rssi, int txPower) {
   /*
    * From Pravin Uttarwar
    *
    * https://www.quora.com/How-do-I-calculate-distance-in-meters-km-yards-from-rssi-values-in-dBm-of-BLE-in-android
    *
    * RSSI = TxPower - 10 * n * lg(d)
    * n = 2 (in free space)
    *
    * d = 10 ^ ((TxPower - RSSI) / (10 * n))
    */

   return pow((double)10, ((double) txPower - rssi) / (10 * 2));
}

void setup_beacon(esp_bd_addr_t bda,int txPower,char *name) {
	uint8_t pos=0;
	uint8_t is_bda=0;
	while (blufi_beacons[pos].pos != 0 && pos < BLUFI_MAX_BEACON) {
		is_bda=0;
		for (int i=0;i<ESP_BD_ADDR_LEN;i++) {
			ESP_LOGD(GAP_TAG, "bda[%d]=%02X blufi_beacons[%d].bda[%d]=%02X %d", i,bda[i],pos,i,blufi_beacons[pos].bda[i],is_bda);
			if (bda[i]==blufi_beacons[pos].bda[i]) {
				is_bda++;
			}
		}
		if (is_bda==ESP_BD_ADDR_LEN) {
			break; // beacon found
		}
		pos++;
	}
	if (is_bda<ESP_BD_ADDR_LEN) {
		ESP_LOGI(GAP_TAG, "blufi_beacons new setup [%d],[%s]", pos,name);
		blufi_beacons[pos].pos = pos+1;
		memcpy(&blufi_beacons[pos].bda, bda, ESP_BD_ADDR_LEN);
		memcpy(&blufi_beacons[pos].name,name, strlen(name)<BLUFI_NAME_LEN?strlen(name):BLUFI_NAME_LEN-1);
		blufi_beacons[pos].txPower=txPower;
	}
}

void add_beacon(esp_bd_addr_t bda,int rssi) {
	uint8_t pos=0;
	uint8_t is_bda=0;
	double distance=0,avgdist=0;
	while (blufi_beacons[pos].pos != 0 && pos < BLUFI_MAX_BEACON) {
		is_bda=0;
		for (int i=0;i<ESP_BD_ADDR_LEN;i++) {
			ESP_LOGD(GAP_TAG, "bda[%d]=%02X blufi_beacons[%d].bda[%d]=%02X %d", i,bda[i],pos,i,blufi_beacons[pos].bda[i],is_bda);
			if (bda[i]==blufi_beacons[pos].bda[i]) {
				is_bda++;
			}
		}
		if (is_bda==ESP_BD_ADDR_LEN) {
			break; // beacon found
		}
		pos++;
	}
	if (is_bda<ESP_BD_ADDR_LEN) {
		ESP_LOGI(GAP_TAG, "blufi_beacons new [%d]", pos);
		blufi_beacons[pos].pos = pos+1;
		memcpy(&blufi_beacons[pos].bda, bda, ESP_BD_ADDR_LEN);
		if (blufi_beacons[pos].txPower==0) {
			blufi_beacons[pos].txPower=-60;  // arbitrarily value
		}
		if (blufi_beacons[pos].name[0]==0) { // no Name
			sprintf(blufi_beacons[pos].name,"%02X%02X%02X%02X%02X%02X",bda[0],bda[1],bda[2],bda[3],bda[4],bda[5]);
		}
	} else {
		ESP_LOGI(GAP_TAG, "blufi_beacons found [%d] [%s]", pos,blufi_beacons[pos].name);
	}
	distance = getDistance(rssi, blufi_beacons[pos].txPower);
	ESP_LOGD(GAP_TAG, "blufi_beacons dist [%d,%d->%lf]", pos,blufi_beacons[pos].actdist,distance);
	blufi_beacons[pos].dist[blufi_beacons[pos].actdist] = distance;
	blufi_beacons[pos].actdist++;
	if (blufi_beacons[pos].actdist>BLUFI_DISTANCE_COUNT) {
		blufi_beacons[pos].actdist=0; // round robin
	}
	blufi_beacons[pos].maxdist++;
	if (blufi_beacons[pos].maxdist>BLUFI_DISTANCE_COUNT) {
		blufi_beacons[pos].maxdist=BLUFI_DISTANCE_COUNT;
	}
	for (int i=0;i<blufi_beacons[pos].maxdist;i++) {
		avgdist += blufi_beacons[pos].dist[i];
		ESP_LOGD(GAP_TAG, "blufi_beacons avgdist [%d,%d->%lf]", pos,i,blufi_beacons[pos].dist[i]);
	}
	ESP_LOGI(GAP_TAG, "blufi_beacons all avgdist [%d->%lf]", pos,avgdist/blufi_beacons[pos].maxdist);
	blufi_beacons[pos].avgdist=avgdist/blufi_beacons[pos].maxdist;

}

uint8_t count_beacon(void) {
	uint8_t pos=0, count=0;
	while (blufi_beacons[pos].pos != 0 && pos < BLUFI_MAX_BEACON) {
		if (blufi_beacons[pos].avgdist>0) {
			count++;
		}
		pos++;
	}
	ESP_LOGI(GAP_TAG, "count_beacon [%d]", count);

	return count;
}


void gap_init(void)
{
    uint32_t duration = 10;
    gapcount = 0;
    memset(&blufi_beacons, 0, sizeof(blufi_beacon_t)*BLUFI_MAX_BEACON);
    setup_beacon((esp_bd_addr_t) { 0xE6,0x3F,0xEC,0xF3,0x7C,0x30 },-62,"Alice");
    setup_beacon((esp_bd_addr_t) { 0xC7,0x15,0xF0,0x6A,0x08,0xD0 },-63,"Mary");
    setup_beacon((esp_bd_addr_t) { 0xCC,0x58,0x7D,0xD2,0x5F,0xAE },-66,"Louis");
    setup_beacon((esp_bd_addr_t) { 0xF9,0xEA,0xE5,0x5F,0x99,0x28 },-63,"Dolly");
    setup_beacon((esp_bd_addr_t) { 0xD8,0x3E,0xFC,0x41,0xEC,0x58 },-57,"Bob");
    setup_beacon((esp_bd_addr_t) { 0xDA,0xF7,0x02,0x60,0x82,0x42 },-63,"Dave");
    setup_beacon((esp_bd_addr_t) { 0xDF,0x15,0x82,0x26,0x75,0xA2 },-60,"Matha");
    setup_beacon((esp_bd_addr_t) { 0x24,0x0A,0xC4,0x01,0xD1,0xCA },-65,"Oscar");
	ESP_LOGI(GAP_TAG, "esp_ble_gap_start_scanning %d", duration);
    esp_ble_gap_start_scanning(duration);

   // esp_ble_gap_set_device_name(BLUFI_DEVICE_NAME);
   // esp_ble_gap_config_adv_data(&blufi_adv_data);
}

void gap_start(void)
{
	// esp_ble_gap_start_advertising(&blufi_adv_params);
}

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	ESP_LOGI(GAP_TAG, "esp_gap_cb, event = %x", event);
    switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        //the unit of the duration is second
        break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        uint8_t is_beacon;
    	ESP_LOGI(GAP_TAG, "search_evt = %x", scan_result->scan_rst.search_evt);
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            ESP_LOGI(GAP_TAG, "BDA %02X,%02X,%02X,%02X,%02X,%02X:",scan_result->scan_rst.bda[0],
            		scan_result->scan_rst.bda[1],scan_result->scan_rst.bda[2],
					scan_result->scan_rst.bda[3],scan_result->scan_rst.bda[4],
					scan_result->scan_rst.bda[5]);
            for (int i = 0; i < 6; i++) {
                server_dba[i]=scan_result->scan_rst.bda[i];
            }
			ESP_LOGD(GAP_TAG, "ble_adv: %02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X", scan_result->scan_rst.ble_adv[0],
					 scan_result->scan_rst.ble_adv[1], scan_result->scan_rst.ble_adv[2], scan_result->scan_rst.ble_adv[3],
					 scan_result->scan_rst.ble_adv[4], scan_result->scan_rst.ble_adv[5], scan_result->scan_rst.ble_adv[6],
					 scan_result->scan_rst.ble_adv[7], scan_result->scan_rst.ble_adv[8], scan_result->scan_rst.ble_adv[9],
					 scan_result->scan_rst.ble_adv[10], scan_result->scan_rst.ble_adv[11], scan_result->scan_rst.ble_adv[12],
					 scan_result->scan_rst.ble_adv[13], scan_result->scan_rst.ble_adv[14], scan_result->scan_rst.ble_adv[15]);
			ESP_LOGD(GAP_TAG, "ble_adv: %02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X", scan_result->scan_rst.ble_adv[16],
					 scan_result->scan_rst.ble_adv[17], scan_result->scan_rst.ble_adv[18], scan_result->scan_rst.ble_adv[19],
					 scan_result->scan_rst.ble_adv[20], scan_result->scan_rst.ble_adv[21], scan_result->scan_rst.ble_adv[22],
					 scan_result->scan_rst.ble_adv[23], scan_result->scan_rst.ble_adv[24], scan_result->scan_rst.ble_adv[25],
					 scan_result->scan_rst.ble_adv[26], scan_result->scan_rst.ble_adv[27], scan_result->scan_rst.ble_adv[28],
					 scan_result->scan_rst.ble_adv[29], scan_result->scan_rst.ble_adv[30], scan_result->scan_rst.ble_adv[31]);
			ESP_LOGI(GAP_TAG, "rssi: %d", scan_result->scan_rst.rssi);
			ESP_LOGD(GAP_TAG, "dist: %lf", getDistance(scan_result->scan_rst.rssi,-61));
			ESP_LOGD(GAP_TAG, "flag: %d", scan_result->scan_rst.flag);
			ESP_LOGD(GAP_TAG, "num_resps: %d", scan_result->scan_rst.num_resps);
			ESP_LOGD(GAP_TAG, "dev_type: %d", scan_result->scan_rst.dev_type);
			ESP_LOGD(GAP_TAG, "ble_addr_type: %d", scan_result->scan_rst.ble_addr_type);
			is_beacon=1;
			for (int i=0;i<BLUFI_BEACON_START_LEN;i++) {
				if (scan_result->scan_rst.ble_adv[i]!=blufi_beacon_start[i]) {
					is_beacon=0;
					break;
				}
			}
			if (is_beacon==1) {
				add_beacon(scan_result->scan_rst.bda,scan_result->scan_rst.rssi);
			}
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
        {
			static char parambuf[128];
			uint8_t bcnt = count_beacon();
			sprintf(parambuf,"field1=%d&field2=%lf&field3=%lf&field4=%lf&field5=%lf&field6=%lf&field7=%lf&field8=%lf",bcnt,
					blufi_beacons[0].avgdist,blufi_beacons[1].avgdist,blufi_beacons[2].avgdist,blufi_beacons[3].avgdist,
					blufi_beacons[4].avgdist,blufi_beacons[5].avgdist,blufi_beacons[6].avgdist);
		    ESP_LOGI(GAP_TAG, "blufi_beacons : %s", parambuf);
        	xTaskCreate(&https_get_task, "https_get_task", 8192, parambuf, 5, NULL);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}
