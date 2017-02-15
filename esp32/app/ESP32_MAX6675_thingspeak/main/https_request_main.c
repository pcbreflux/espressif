/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/base64.h"
#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "driver/spi_master.h"

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "api.thingspeak.com"
#define WEB_PORT "443"
#define WEB_URL "https://api.thingspeak.com/update.json?api_key=YOURAPIKEY&field1="

static const char *TAG = "example";

//#define MAX6675_MISO 19
//#define MAX6675_SCK 18
//#define MAX6675_CS 5
//#define MAX6675_SPI HOST VSPI_HOST
#define MAX6675_MISO 12
#define MAX6675_SCK 14
#define MAX6675_CS 15
#define MAX6675_SPI_HOST HSPI_HOST
#define GPIO_OUTPUT_PIN_MASK  ((1<<MAX6675_CS))

#ifndef HIGH
    #define HIGH 1
#endif
#ifndef LOW
    #define LOW 0
#endif

static spi_device_handle_t spi_handle; // SPI handle.

/* Root cert for howsmyssl.com, found in cert.c */
extern const char *server_root_cert;

#ifdef MBEDTLS_DEBUG_C

#define MBEDTLS_DEBUG_LEVEL 4

/* mbedtls debug function that translates mbedTLS debug output
   to ESP_LOGx debug output.

   MBEDTLS_DEBUG_LEVEL 4 means all mbedTLS debug output gets sent here,
   and then filtered to the ESP logging mechanism.
*/
static void mbedtls_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str)
{
    const char *MBTAG = "mbedtls";
    char *file_sep;

    /* Shorten 'file' from the whole file path to just the filename

       This is a bit wasteful because the macros are compiled in with
       the full _FILE_ path in each case.
    */
    file_sep = rindex(file, '/');
    if(file_sep)
        file = file_sep+1;

    switch(level) {
    case 1:
        ESP_LOGI(MBTAG, "%s:%d %s", file, line, str);
        break;
    case 2:
    case 3:
        ESP_LOGD(MBTAG, "%s:%d %s", file, line, str);
    case 4:
        ESP_LOGV(MBTAG, "%s:%d %s", file, line, str);
        break;
    default:
        ESP_LOGE(MBTAG, "Unexpected log level %d: %s", level, str);
        break;
    }
}

#endif

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
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void init_gpio() {
	gpio_config_t io_conf;

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE; //disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT; //set as output mode
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_MASK; //bit mask of the pins
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; //disable pull-down mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;   //disable pull-up mode
    ESP_LOGD(TAG, "gpio_config");
    gpio_config(&io_conf); //configure GPIO with the given settings

}

uint16_t readMax6675() {
	uint16_t data,rawtemp,temp=0;
    spi_bus_config_t bus_config;
	spi_device_interface_config_t dev_config;
	spi_transaction_t trans_word;

	ESP_LOGD(TAG, "readMax6675 start");

    gpio_set_level(MAX6675_CS, HIGH); // MAX6675_CS
	bus_config.sclk_io_num   = MAX6675_SCK; // CLK
	bus_config.mosi_io_num   = -1; // MOSI not used
	bus_config.miso_io_num   = MAX6675_MISO; // MISO
	bus_config.quadwp_io_num = -1; // not used
	bus_config.quadhd_io_num = -1; // not used
    ESP_LOGD(TAG, "spi_bus_initialize");
	ESP_ERROR_CHECK(spi_bus_initialize(MAX6675_SPI_HOST, &bus_config, 2));

	dev_config.address_bits     = 0;
	dev_config.command_bits     = 0;
	dev_config.dummy_bits       = 0;
	dev_config.mode             = 0; // SPI_MODE0
	dev_config.duty_cycle_pos   = 0;
	dev_config.cs_ena_posttrans = 0;
	dev_config.cs_ena_pretrans  = 0;
	//dev_config.clock_speed_hz   = 2000000;  // 2 MHz
	dev_config.clock_speed_hz   = 10000;  // 10 kHz
	dev_config.spics_io_num     = -1; // CS External
	dev_config.flags            = 0; // SPI_MSBFIRST
	dev_config.queue_size       = 100;
	dev_config.pre_cb           = NULL;
	dev_config.post_cb          = NULL;
    ESP_LOGD(TAG, "spi_bus_add_device");
	ESP_ERROR_CHECK(spi_bus_add_device(MAX6675_SPI_HOST, &dev_config, &spi_handle));


    ESP_LOGD(TAG, "MAX6675_CS prepare");
    gpio_set_level(MAX6675_CS, LOW); // MAX6675_CS prepare
    vTaskDelay(500 / portTICK_RATE_MS);  // see MAX6675 datasheet

    rawtemp = 0x000;
    data = 0x000;  // write dummy

	trans_word.address   = 0;
	trans_word.command   = 0;
	trans_word.flags     = 0;
	trans_word.length    = 8 * 2; // Total data length, in bits NOT number of bytes.
	trans_word.rxlength  = 0; // (0 defaults this to the value of ``length``)
	trans_word.tx_buffer = &data;
	trans_word.rx_buffer = &rawtemp;
    ESP_LOGD(TAG, "spi_device_transmit");
	ESP_ERROR_CHECK(spi_device_transmit(spi_handle, &trans_word));

	gpio_set_level(MAX6675_CS, HIGH); // MAX6675_CS prepare

    temp = ((((rawtemp & 0x00FF) << 8) | ((rawtemp & 0xFF00) >> 8))>>3)*25;
   // temp = ((rawtemp)>>3)*25;
	ESP_LOGI(TAG, "readMax6675 spiReadWord=%x temp=%d.%d",rawtemp,temp/100,temp%100);

    ESP_LOGD(TAG, "spi_bus_remove_device");
    ESP_ERROR_CHECK(spi_bus_remove_device(spi_handle));
    ESP_LOGD(TAG, "spi_bus_free");
    ESP_ERROR_CHECK(spi_bus_free(MAX6675_SPI_HOST));

    return temp;

}

/*
static void temp_get_task(void *pvParameters) {
    uint16_t temp;

    while(1) {
		temp=readMax6675();
		// ESP_LOGI(TAG, "temp=%d.%d",temp/100,temp%100);

		vTaskDelay(5000 / portTICK_RATE_MS);
		ESP_LOGD(TAG, "Starting again!");
    }

    vTaskDelete(NULL);
}
*/

static void https_get_task(void *pvParameters) {
    char buf[512];
    int ret, flags, len;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    ESP_LOGI(TAG, "Seeding the random number generator");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                    NULL, 0)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        abort();
    }

    ESP_LOGI(TAG, "Loading the CA root certificate...");

    ret = mbedtls_x509_crt_parse(&cacert, (uint8_t*)server_root_cert, strlen(server_root_cert)+1);
    if(ret < 0)
    {
        ESP_LOGE(TAG, "mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting hostname for TLS session...");

     /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

    if((ret = mbedtls_ssl_config_defaults(&conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.

       You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
    */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#ifdef MBEDTLS_DEBUG_C
    mbedtls_debug_set_threshold(MBEDTLS_DEBUG_LEVEL);
    mbedtls_ssl_conf_dbg(&conf, mbedtls_debug, NULL);
#endif

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

    while(1) {
        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                            false, true, portMAX_DELAY);
        ESP_LOGI(TAG, "Connected to AP");

        mbedtls_net_init(&server_fd);

        ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

        if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                      WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
            goto exit;
        }

        ESP_LOGI(TAG, "Connected.");

        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");

        while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
                goto exit;
            }
        }

        ESP_LOGI(TAG, "Verifying peer X.509 certificate...");

        if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
        {
            /* In real life, we probably want to close connection if ret != 0 */
            ESP_LOGW(TAG, "Failed to verify peer certificate!");
            bzero(buf, sizeof(buf));
            mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
            ESP_LOGW(TAG, "verification info: %s", buf);
        }
        else {
            ESP_LOGI(TAG, "Certificate verified.");
        }

        ESP_LOGI(TAG, "Writing HTTP request...");

        uint16_t temp;
        temp=readMax6675();

         ESP_LOGI(TAG, "temp=%d",temp);

         if (temp!=0) {

			 char reqbuf[512];

			 sprintf(reqbuf,"GET %s%d.%d HTTP/1.1\nHost: %s\nUser-Agent: esp-idf/1.0 esp32\n\n",WEB_URL,temp/100,temp%100,WEB_SERVER);

			 ESP_LOGI(TAG, "req=[%s]",reqbuf);

			while((ret = mbedtls_ssl_write(&ssl, (const unsigned char *)reqbuf, strlen(reqbuf))) <= 0)
			{
				if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				{
					ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
					goto exit;
				}
			}

			len = ret;
			ESP_LOGI(TAG, "%d bytes written", len);
			ESP_LOGI(TAG, "Reading HTTP response...");

			do
			{
				len = sizeof(buf) - 1;
				bzero(buf, sizeof(buf));
				ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

				if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
					continue;

				if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
					ret = 0;
					break;
				}

				if(ret < 0)
				{
					ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
					break;
				}

				if(ret == 0)
				{
					ESP_LOGI(TAG, "connection closed");
					break;
				}

				len = ret;
				ESP_LOGI(TAG, "%d bytes read", len);
				/* Print response directly to stdout as it is read */
				for(int i = 0; i < len; i++) {
					putchar(buf[i]);
				}
			} while(1);
         }
        mbedtls_ssl_close_notify(&ssl);

    exit:
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);

        if(ret != 0)
        {
            mbedtls_strerror(ret, buf, 100);
            ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
        }

        for(int countdown = 60; countdown >= 0; countdown--) {
        	if(countdown%10==0) {
        		ESP_LOGI(TAG, "%d...", countdown);
        	}
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main() {
    nvs_flash_init();
    initialise_wifi();
    init_gpio();

	//xTaskCreate(&temp_get_task, "temp_get_task", 8192, NULL, 5, NULL);
	xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
}
