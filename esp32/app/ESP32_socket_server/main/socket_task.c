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

/*
 * this example based on code from Neil Kolban - big thanks for that
 * see https://github.com/nkolban/esp32-snippets/tree/master/sockets
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "lwip/sockets.h"

#include "sdkconfig.h"

#include "socket_task.h"

#define TAG "SOCKET"
#define PORT_NUMBER 8001


extern EventGroupHandle_t wifi_event_group;
extern const int CONNECTED_BIT;
SemaphoreHandle_t clientcount_semaphore;


void sendData(int clientSock) {
	char msg[255];
	int ret;

	for (int pos=0;pos<10000000;pos++) {
		sprintf(msg,"Hello from ESP32 %d\r\n",pos);
		ret = send(clientSock, msg, strlen(msg), 0);
		if (ret < 0) {
			ESP_LOGE(TAG, "send: %d %s (%d)", ret, strerror(errno),errno);
			break;
		}
		vTaskDelay(1/portTICK_PERIOD_MS);
	}
}
void client_task(void *pvParameters) {
	int clientSock = (int)pvParameters;
	ESP_LOGI(TAG, "I am Client No: %u", uxQueueMessagesWaiting(clientcount_semaphore));

	sendData(clientSock);
	close(clientSock);
	if(xSemaphoreTake(clientcount_semaphore,100/portTICK_RATE_MS)!=pdTRUE) {  // max wait 100ms
		ESP_LOGE(TAG, "client_task fail to receive semaphore");
	} else {
		ESP_LOGI(TAG, "Clients Left: %u", uxQueueMessagesWaiting(clientcount_semaphore));
	}
	vTaskDelete(NULL);
}

void socket_task(void *pvParameters) {
	struct sockaddr_in clientAddress;
	struct sockaddr_in serverAddress;

    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");

	// Create a socket that we will listen upon.
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		ESP_LOGE(TAG, "socket: %d %s", sock, strerror(errno));
		goto END;
	}
	clientcount_semaphore=xSemaphoreCreateCounting(255,0);

	// Bind our server socket to a port.
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT_NUMBER);
	int rc  = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc < 0) {
		ESP_LOGE(TAG, "bind: %d %s", rc, strerror(errno));
		goto END;
	}

	// Flag the socket as listening for new connections.
	rc = listen(sock, 10);
	if (rc < 0) {
		ESP_LOGE(TAG, "listen: %d %s", rc, strerror(errno));
		goto END;
	}

	while (1) {

		// Listen for a new client connection.
		socklen_t clientAddressLength = sizeof(clientAddress);
		int clientSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (clientSock < 0) {
			ESP_LOGE(TAG, "accept: %d %s (%d)", clientSock, strerror(errno),errno);
			if (errno!=23) {
				goto END;
			}
		}
		xSemaphoreGive(clientcount_semaphore);
        xTaskCreate(&client_task,"client_task",2048,(void *)clientSock, 5, NULL);

	}
	END:
	vTaskDelete(NULL);
}
