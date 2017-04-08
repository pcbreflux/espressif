/* MQTT Example using plain mbedTLS sockets
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2017 pcbreflux, Apache 2.0 License.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

/* Constants that aren't configurable in menuconfig */
#define MQTT_SERVER "m20.cloudmqtt.com"
#define MQTT_USER "user"
#define MQTT_PASS "password"
//#define MQTT_PORT 1883
//#define MQTT_PORT 24352
#define MQTT_PORT 34352
// #define MQTT_WEBSOCKET 0  // 0=no 1=yes
#define MQTT_WEBSOCKET 1  // 0=no 1=yes
#define MQTT_BUF_SIZE 512

#define GPIO_OUTPUT    13

#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880

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
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void play_march(uint8_t longplay);  // Forward declaration

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void mqtt_message_handler(MessageData *md) {
	ESP_LOGI(TAG, "Topic received!: %.*s %.*s", md->topicName->lenstring.len, md->topicName->lenstring.data, md->message->payloadlen, (char*)md->message->payload);
	if (strncmp((const char *)md->message->payload,"march",5)==0) {
		play_march(1);
	}
}

#pragma GCC diagnostic pop

void sound(int gpio_num,uint32_t freq,uint32_t duration) {

	ledc_timer_config_t timer_conf;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.bit_num    = LEDC_TIMER_10_BIT;
	timer_conf.timer_num  = LEDC_TIMER_0;
	timer_conf.freq_hz    = freq;
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.gpio_num   = gpio_num;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.channel    = LEDC_CHANNEL_0;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.timer_sel  = LEDC_TIMER_0;
	ledc_conf.duty       = 0x0; // 50%=0x3FFF, 100%=0x7FFF for 15 Bit
	                            // 50%=0x01FF, 100%=0x03FF for 10 Bit
	ledc_channel_config(&ledc_conf);

	// start
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0x7F); // 12% duty - play here for your speaker or buzzer
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
	vTaskDelay(duration/portTICK_PERIOD_MS);
	// stop
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

}

// based on https://wiki.mikrotik.com/wiki/Super_Mario_Theme
void play_theme() {
	sound(GPIO_OUTPUT,660,100);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,510,100);
	vTaskDelay(100/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,770,100);
	vTaskDelay(550/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,380,100);
	vTaskDelay(575/portTICK_PERIOD_MS);

	sound(GPIO_OUTPUT,510,100);
	vTaskDelay(450/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,380,100);
	vTaskDelay(400/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,320,100);
	vTaskDelay(500/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,440,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,480,80);
	vTaskDelay(330/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,450,100);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,430,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,380,100);
	vTaskDelay(200/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,80);
	vTaskDelay(200/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,760,50);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,860,100);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,700,80);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,760,50);
	vTaskDelay(350/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,660,80);
	vTaskDelay(300/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,520,80);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,580,80);
	vTaskDelay(150/portTICK_PERIOD_MS);
	sound(GPIO_OUTPUT,480,80);
	vTaskDelay(500/portTICK_PERIOD_MS);
}
// based on http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March#Code
// original composed by John Williams for the film Star Wars: The Empire Strikes Back
void play_march(uint8_t longplay) {

    sound(GPIO_OUTPUT,a, 500);
    sound(GPIO_OUTPUT,a, 500);
    sound(GPIO_OUTPUT,a, 500);
    sound(GPIO_OUTPUT,f, 350);
    sound(GPIO_OUTPUT,cH, 150);
    sound(GPIO_OUTPUT,a, 500);
    sound(GPIO_OUTPUT,f, 350);
    sound(GPIO_OUTPUT,cH, 150);
    sound(GPIO_OUTPUT,a, 650);

    vTaskDelay(150/portTICK_PERIOD_MS);
    //end of first bit

    sound(GPIO_OUTPUT,eH, 500);
    sound(GPIO_OUTPUT,eH, 500);
    sound(GPIO_OUTPUT,eH, 500);
    sound(GPIO_OUTPUT,fH, 350);
    sound(GPIO_OUTPUT,cH, 150);
    sound(GPIO_OUTPUT,gS, 500);
    sound(GPIO_OUTPUT,f, 350);
    sound(GPIO_OUTPUT,cH, 150);
    sound(GPIO_OUTPUT,a, 650);

    vTaskDelay(150/portTICK_PERIOD_MS);
    //end of second bit...

    sound(GPIO_OUTPUT,aH, 500);
    sound(GPIO_OUTPUT,a, 300);
    sound(GPIO_OUTPUT,a, 150);
    sound(GPIO_OUTPUT,aH, 400);
    sound(GPIO_OUTPUT,gSH, 200);
    sound(GPIO_OUTPUT,gH, 200);
    sound(GPIO_OUTPUT,fSH, 125);
    sound(GPIO_OUTPUT,fH, 125);
    sound(GPIO_OUTPUT,fSH, 250);

    vTaskDelay(250/portTICK_PERIOD_MS);

    sound(GPIO_OUTPUT,aS, 250);
    sound(GPIO_OUTPUT,dSH, 400);
    sound(GPIO_OUTPUT,dH, 200);
    sound(GPIO_OUTPUT,cSH, 200);
    sound(GPIO_OUTPUT,cH, 125);
    sound(GPIO_OUTPUT,b, 125);
    sound(GPIO_OUTPUT,cH, 250);

    vTaskDelay(250/portTICK_PERIOD_MS);

    sound(GPIO_OUTPUT,f, 125);
    sound(GPIO_OUTPUT,gS, 500);
    sound(GPIO_OUTPUT,f, 375);
    sound(GPIO_OUTPUT,a, 125);
    sound(GPIO_OUTPUT,cH, 500);
    sound(GPIO_OUTPUT,a, 375);
    sound(GPIO_OUTPUT,cH, 125);
    sound(GPIO_OUTPUT,eH, 650);

    //end of third bit... (Though it doesn't play well)
    //let's repeat it
    if (longplay>=1) {
		sound(GPIO_OUTPUT,aH, 500);
		sound(GPIO_OUTPUT,a, 300);
		sound(GPIO_OUTPUT,a, 150);
		sound(GPIO_OUTPUT,aH, 400);
		sound(GPIO_OUTPUT,gSH, 200);
		sound(GPIO_OUTPUT,gH, 200);
		sound(GPIO_OUTPUT,fSH, 125);
		sound(GPIO_OUTPUT,fH, 125);
		sound(GPIO_OUTPUT,fSH, 250);

		vTaskDelay(250/portTICK_PERIOD_MS);

		sound(GPIO_OUTPUT,aS, 250);
		sound(GPIO_OUTPUT,dSH, 400);
		sound(GPIO_OUTPUT,dH, 200);
		sound(GPIO_OUTPUT,cSH, 200);
		sound(GPIO_OUTPUT,cH, 125);
		sound(GPIO_OUTPUT,b, 125);
		sound(GPIO_OUTPUT,cH, 250);

		vTaskDelay(250/portTICK_PERIOD_MS);

		sound(GPIO_OUTPUT,f, 250);
		sound(GPIO_OUTPUT,gS, 500);
		sound(GPIO_OUTPUT,f, 375);
		sound(GPIO_OUTPUT,cH, 125);
		sound(GPIO_OUTPUT,a, 500);
		sound(GPIO_OUTPUT,f, 375);
		sound(GPIO_OUTPUT,cH, 125);
		sound(GPIO_OUTPUT,a, 650);
		//end of the song
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
		ret = MQTTSubscribe(&client, "esp32/sound", QOS0, mqtt_message_handler);
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
