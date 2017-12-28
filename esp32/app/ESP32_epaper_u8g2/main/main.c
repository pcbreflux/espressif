/** @file main.c
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
 * @brief Application main file.
 *
*/
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"

void task_test_gde0213b1(void *ignore);
void task_test_gdew0154t8(void *ignore);
void task_test_gdep015oc1(void *ignore);

/**@brief test u8g2 displays with tasks.
 */
void app_main() {
//  xTaskCreate(&task_test_gde0213b1, "task_test_gde0213b1", 8192, NULL, 5, NULL);
 	xTaskCreate(&task_test_gdew0154t8, "task_test_gdew0154t8", 8192, NULL, 5, NULL);
//  xTaskCreate(&task_test_gdep015oc1, "task_test_gdep015oc1", 8192, NULL, 5, NULL);
}
