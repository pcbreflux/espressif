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
#ifndef BH1750_H_
#define BH1750_H_

#include "bh1750.h"
#define BH1750_ADDRESS1	0x23
#define BH1750_ADDRESS2	0x5C
#define BH1750_POWER_DOWN 0x00
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_CONTINUOUS_HIGH_RES_MODE 0x10
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2 0x11
#define BH1750_CONTINUOUS_LOW_RES_MODE 0x13
#define BH1750_ONE_TIME_HIGH_RES_MODE 0x20
#define BH1750_ONE_TIME_HIGH_RES_MODE_2 0x21
#define BH1750_ONE_TIME_LOW_RES_MODE 0x23

void bh1750_init(void);
float bh1750_read(void);
void bh1750_deinit(void);

#endif /* BH1750_H_ */
