/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2016-2017 pcbreflux, Apache 2.0 License.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_deep_sleep.h"

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

#include "esp_blufi_api.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "blufi_beacon_scan.h"
#include "blufi_https_request.h"

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int boot_count = 0;

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "api.thingspeak.com"
#define WEB_PORT "443"
#define WEB_URL "https://api.thingspeak.com/update.json?api_key=VNOFJH1DQHUCAC5Z&"

#define HTTPS_TAG "BEACON_HTTPS"
/* Root cert for howsmyssl.com, found in cert.c */
extern const char *server_root_cert;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
extern EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int HTTPS_CONNECTED_BIT = BIT0;

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

void https_get_task(void * pvParameters)
{
    char buf[512];
    int ret, flags, len;
    char *param=(char *)pvParameters;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    ESP_LOGI(HTTPS_TAG, "param: [%s]", param);
    ++boot_count;
    ESP_LOGI(HTTPS_TAG, "Boot count: %d", boot_count);
    ESP_LOGI(HTTPS_TAG, "mbedtls_ssl_init...");
    mbedtls_ssl_init(&ssl);
    ESP_LOGI(HTTPS_TAG, "mbedtls_x509_crt_init...");
    mbedtls_x509_crt_init(&cacert);
    ESP_LOGI(HTTPS_TAG, "mbedtls_ctr_drbg_init...");
    mbedtls_ctr_drbg_init(&ctr_drbg);
    ESP_LOGI(HTTPS_TAG, "Seeding the random number generator");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                    NULL, 0)) != 0)
    {
        ESP_LOGE(HTTPS_TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        abort();
    }

    ESP_LOGI(HTTPS_TAG, "Loading the CA root certificate...");

    ret = mbedtls_x509_crt_parse(&cacert, (uint8_t*)server_root_cert, strlen(server_root_cert)+1);
    if(ret < 0)
    {
        ESP_LOGE(HTTPS_TAG, "mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        abort();
    }

    ESP_LOGI(HTTPS_TAG, "Setting hostname for TLS session...");

     /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
        ESP_LOGE(HTTPS_TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        abort();
    }

    ESP_LOGI(HTTPS_TAG, "Setting up the SSL/TLS structure...");

    if((ret = mbedtls_ssl_config_defaults(&conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ESP_LOGE(HTTPS_TAG, "mbedtls_ssl_config_defaults returned %d", ret);
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
        ESP_LOGE(HTTPS_TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

    while(1) {
        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
        xEventGroupWaitBits(wifi_event_group, HTTPS_CONNECTED_BIT,
                            false, true, portMAX_DELAY);
        ESP_LOGI(HTTPS_TAG, "Connected to AP");

        mbedtls_net_init(&server_fd);

        ESP_LOGI(HTTPS_TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

        if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                      WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            ESP_LOGE(HTTPS_TAG, "mbedtls_net_connect returned -%x", -ret);
            goto exit;
        }

        ESP_LOGI(HTTPS_TAG, "Connected.");

        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        ESP_LOGI(HTTPS_TAG, "Performing the SSL/TLS handshake...");

        while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                ESP_LOGE(HTTPS_TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
                goto exit;
            }
        }

        ESP_LOGI(HTTPS_TAG, "Verifying peer X.509 certificate...");

        if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
        {
            /* In real life, we probably want to close connection if ret != 0 */
            ESP_LOGW(HTTPS_TAG, "Failed to verify peer certificate!");
            bzero(buf, sizeof(buf));
            mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
            ESP_LOGW(HTTPS_TAG, "verification info: %s", buf);
        }
        else {
            ESP_LOGI(HTTPS_TAG, "Certificate verified.");
        }

        ESP_LOGI(HTTPS_TAG, "Writing HTTP request...");

         if (param!=NULL) {

			char reqbuf[1024];

			sprintf(reqbuf,"GET %s%s HTTP/1.1\nHost: %s\nUser-Agent: esp-idf/1.0 esp32\n\n",WEB_URL,param,WEB_SERVER);

			 ESP_LOGI(HTTPS_TAG, "req=[%s]",reqbuf);

			while((ret = mbedtls_ssl_write(&ssl, (const unsigned char *)reqbuf, strlen(reqbuf))) <= 0)
			{
				if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				{
					ESP_LOGE(HTTPS_TAG, "mbedtls_ssl_write returned -0x%x", -ret);
					goto exit;
				}
			}

			len = ret;
			ESP_LOGI(HTTPS_TAG, "%d bytes written", len);
			ESP_LOGI(HTTPS_TAG, "Reading HTTP response...");

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
					ESP_LOGE(HTTPS_TAG, "mbedtls_ssl_read returned -0x%x", -ret);
					break;
				}

				if(ret == 0)
				{
					ESP_LOGI(HTTPS_TAG, "connection closed");
					break;
				}

				len = ret;
				ESP_LOGI(HTTPS_TAG, "%d bytes read", len);
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
            ESP_LOGE(HTTPS_TAG, "Last error was: -0x%x - %s", -ret, buf);
        }

        for(int countdown = 10; countdown >= 0; countdown--) {
        		ESP_LOGI(HTTPS_TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
        const int deep_sleep_sec = 30;
        ESP_LOGI(HTTPS_TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
        esp_deep_sleep(1000000LL * deep_sleep_sec);
    }
}
