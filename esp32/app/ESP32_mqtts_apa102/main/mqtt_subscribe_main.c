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
#include <string.h>
#include <stdlib.h>

#include "sdkconfig.h"

#include "MQTTClient.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "smart_led.h"

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

/* Constants that aren't configurable in menuconfig */
#define MQTT_SERVER "192.168.43.152"
#define MQTT_USER "sensor1"
#define MQTT_PASS "sensor1"
#define MQTT_PORT 9883
//#define MQTT_PORT 24352
//#define MQTT_PORT 34352
//#define MQTT_WEBSOCKET 0  // 0=no 1=yes
#define MQTT_WEBSOCKET 1  // 0=no 1=yes
#define MQTT_BUF_SIZE 512

static unsigned char mqtt_sendBuf[MQTT_BUF_SIZE];
static unsigned char mqtt_readBuf[MQTT_BUF_SIZE];

static const char *TAG = "MQTTS";

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}
//            .ssid = CONFIG_WIFI_SSID,
//            .password = CONFIG_WIFI_PASSWORD,

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

uint8_t hexstr2unit8(uint8_t *out, const char *s) {
    char *end;
    if (s[0] == '\0')
        return 1;

    long l = strtol(s, &end, 16);
	ESP_LOGI(TAG, "hexstr2unit8: %s -> %ld",s, l);
    if (l > 255)
        return 1;
    if (l < 0)
        return 1;
    if (*end != '\0')
        return 1;
    *out = (uint8_t)l;
    return 0;
}
static void led_task(void *pvParameters) {
	colorRGBB *pcolor=(colorRGBB *)pvParameters;

	set_smart_led(pcolor->red, pcolor->green, pcolor->blue, pcolor->brightness);

	vTaskDelete(NULL);
}

static void mqtt_message_handler(MessageData *md) {
	uint8_t ret;
	char hexcolor[3];
	colorRGBB color;

	ESP_LOGI(TAG, "Topic received!: %.*s %.*s", md->topicName->lenstring.len, md->topicName->lenstring.data, md->message->payloadlen, (char*)md->message->payload);
	if (md->message->payloadlen>=7 && ((char *)md->message->payload)[0]=='#') {
		hexcolor[0]=(char)((char *)md->message->payload)[1];
		hexcolor[1]=(char)((char *)md->message->payload)[2];
		hexcolor[2]='\0';
		ret=hexstr2unit8(&color.red,hexcolor);
		if (ret!=0) {
			color.red=0;
		}
		hexcolor[0]=(char)((char *)md->message->payload)[3];
		hexcolor[1]=(char)((char *)md->message->payload)[4];
		hexcolor[2]='\0';
		ret=hexstr2unit8(&color.green,hexcolor);
		if (ret!=0) {
			color.green=0;
		}
		hexcolor[0]=(char)((char *)md->message->payload)[5];
		hexcolor[1]=(char)((char *)md->message->payload)[6];
		hexcolor[2]='\0';
		ret=hexstr2unit8(&color.blue,hexcolor);
		if (ret!=0) {
			color.blue=0;
		}
		color.brightness = ((((uint32_t)color.red+(uint32_t)color.red+(uint32_t)color.red+(uint32_t)color.blue+(uint32_t)color.green+(uint32_t)color.green+(uint32_t)color.green+(uint32_t)color.green)>>6)&0x1F);
		//brightness=31;
        xTaskCreate(&led_task,"led_task",2048,(void *)&color, 5, NULL);
	} else if (md->message->payloadlen>=1 && ((char *)md->message->payload)[0]=='t') {
		color.red = 0;
		color.green = 0;
		color.blue = 0;
		color.brightness = 255;
        xTaskCreate(&led_task,"led_task",2048,(void *)&color, 5, NULL);

	}
}


static void mqtt_task(void *pvParameters)
{
	int ret;
	Network network;

    while(1) {
		ESP_LOGD(TAG,"Wait for WiFi ...");
        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                            false, true, portMAX_DELAY);
        ESP_LOGD(TAG, "Connected to AP");

		ESP_LOGD(TAG, "Start MQTT Task ...");

		MQTTClient client;
		NetworkInit(&network);
		network.websocket = MQTT_WEBSOCKET;

		ESP_LOGD(TAG,"NetworkConnect %s:%d ...",MQTT_SERVER,MQTT_PORT);
		ret = NetworkConnect(&network, MQTT_SERVER, MQTT_PORT);
		if (ret != 0) {
			ESP_LOGI(TAG, "NetworkConnect not SUCCESS: %d", ret);
			goto exit;
		}
		ESP_LOGD(TAG,"MQTTClientInit  ...");
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
		ESP_LOGI(TAG,"MQTTClientInit  %s",buf);
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

		ESP_LOGI(TAG,"MQTTConnect  ...");
		ret = MQTTConnect(&client, &data);
		if (ret != SUCCESS) {
			ESP_LOGI(TAG, "MQTTConnect not SUCCESS: %d", ret);
			goto exit;
		}

		ESP_LOGI(TAG, "MQTTSubscribe  ...");
		ret = MQTTSubscribe(&client, "esp32/dev0001/color", QOS1, mqtt_message_handler);
		if (ret != SUCCESS) {
			ESP_LOGI(TAG, "MQTTSubscribe: %d", ret);
			goto exit;
		}
		ESP_LOGI(TAG, "MQTTYield  ...");
		while(1) {
			ret = MQTTYield(&client, (data.keepAliveInterval+1)*1000);
			if (ret != SUCCESS) {
				ESP_LOGI(TAG, "MQTTYield: %d", ret);
				goto exit;
			}
		}
		exit:
			MQTTDisconnect(&client);
			NetworkDisconnect(&network);
			ESP_LOGI(TAG, "Starting again!");
    }
    esp_task_wdt_delete();
    vTaskDelete(NULL);
 }

void app_main()
{
    nvs_flash_init();
    initialise_wifi();
    xTaskCreate(&mqtt_task, "mqtt_task", 12288, NULL, 5, NULL);
}
