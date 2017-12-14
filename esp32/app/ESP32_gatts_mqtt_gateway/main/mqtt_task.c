/*
 * mqtt_task.c
 *
 *  Created on: 09.03.2017
 *      Author: pcbreflux
 */

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
//#include "freertos/heap_regions.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
//#include "esp_heap_alloc_caps.h"

#include "nvs_flash.h"

#include "MQTTClient.h"

#define MQTT_TAG "MQTT"

/* Constants that aren't configurable in menuconfig */
#define MQTT_SERVER "m20.cloudmqtt.com"
#define MQTT_USER "sen1"
#define MQTT_PASS "sen1"
//#define MQTT_PORT 1883
//#define MQTT_PORT 24352
#define MQTT_PORT 34352
//#define MQTT_WEBSOCKET 0  // 0=no 1=yes
#define MQTT_WEBSOCKET 1  // 0=no 1=yes
#define MQTT_BUF_SIZE 512

static unsigned char mqtt_sendBuf[MQTT_BUF_SIZE];
static unsigned char mqtt_readBuf[MQTT_BUF_SIZE];

extern EventGroupHandle_t wifi_event_group;

const int WIFI_CONNECTED_BIT = BIT0;

void mqtt_task(void *pvParameters) {
	int ret;
	Network network;
	MQTTClient client;
    char *param=(char *)pvParameters;

	NetworkInit(&network);
	network.websocket = MQTT_WEBSOCKET;

    ESP_LOGI(MQTT_TAG, "param: [%s]", param);
	/* Wait for the callback to set the CONNECTED_BIT in the
	   event group.
	*/
	xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
	ESP_LOGI(MQTT_TAG, "Connected to AP");


    ESP_LOGI(MQTT_TAG, "Start MQTT Task ...");

//	ESP_LOGI(MQTT_TAG,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
	ESP_LOGI(MQTT_TAG,"NetworkConnect %s:%d ...",MQTT_SERVER,MQTT_PORT);
	NetworkConnect(&network, MQTT_SERVER, MQTT_PORT);
	ESP_LOGI(MQTT_TAG,"MQTTClientInit  ...");
	MQTTClientInit(&client, &network,
		2000,            // command_timeout_ms
		mqtt_sendBuf,         //sendbuf,
		MQTT_BUF_SIZE, //sendbuf_size,
		mqtt_readBuf,         //readbuf,
		MQTT_BUF_SIZE  //readbuf_size
	);

	char buf[30];
	MQTTString clientId = MQTTString_initializer;
#if defined(MBEDTLS_MQTT_DEBUG)
	sprintf(buf, "ESP32MQTT");
#else
	sprintf(buf, "ESP32MQTT%08X",esp_random());
#endif
	ESP_LOGI(MQTT_TAG,"MQTTClientInit  %s",buf);
	clientId.cstring = buf;

	MQTTString username = MQTTString_initializer;
	username.cstring = MQTT_USER;

	MQTTString password = MQTTString_initializer;
	password.cstring = MQTT_PASS;

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.clientID          = clientId;
	data.willFlag          = 0;
	data.MQTTVersion       = 4; // 3 = 3.1 4 = 3.1.1
	data.keepAliveInterval = 5;
	data.cleansession      = 1;
	data.username          = username;
	data.password          = password;

	ESP_LOGI(MQTT_TAG,"MQTTConnect  ...");
	ret = MQTTConnect(&client, &data);
	if (ret != SUCCESS) {
		ESP_LOGI(MQTT_TAG, "MQTTConnect not SUCCESS: %d", ret);
		goto exit;
	}

	ESP_LOGI(MQTT_TAG, "MQTTPublish  ...");
	MQTTMessage message;
	//char msgbuf[100];
	//sprintf(msgbuf, "field1=%d&field2=%lf",(uint8_t)(esp_random()&0xFF),((double)esp_random())/100);

	message.qos = QOS0;
	message.retained = false;
	message.dup = false;
	message.payload = (void*)param;
	message.payloadlen = strlen(param)+1;

	ret = MQTTPublish(&client, "esp32/gateway", &message);
	if (ret != SUCCESS) {
		ESP_LOGI(MQTT_TAG, "MQTTPublish not SUCCESS: %d", ret);
		goto exit;
	}

	exit:
		MQTTDisconnect(&client);
		NetworkDisconnect(&network);

		vTaskDelete(NULL);
 }


