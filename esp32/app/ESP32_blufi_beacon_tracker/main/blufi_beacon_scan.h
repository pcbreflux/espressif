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
#ifndef __BLUFI_BEACON_SCAN_H__
#define __BLUFI_BEACON_SCAN_H__

void gap_init(void);
void gap_start(void);
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

typedef struct blufi_beacon_s blufi_beacon_t;

#define BLUFI_DISTANCE_COUNT 5
#define BLUFI_NAME_LEN 20

struct blufi_beacon_s {
	uint8_t pos;
	esp_bd_addr_t bda;                 /**< Beacon address */
	int txPower;                 /**< Beacon tx_power at 1m */
	char name[BLUFI_NAME_LEN];     /**< Beacon name */
    double dist[BLUFI_DISTANCE_COUNT];               /**< distance */
    uint8_t actdist;             /**< actual distance counter */
    uint8_t maxdist;             /**< max distance counter */
    double avgdist;              /**< average distance */
};

#endif /* __BLUFI_BEACON_SCAN_H__ */
