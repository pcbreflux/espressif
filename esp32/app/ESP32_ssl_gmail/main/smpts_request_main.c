/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the smtp.gmail.com API via TLS v1.2 and reads a SMTPS
 * response.
 *
 * Adapted from the ssl_client1 and ssl_mail_client example in mbedtls.
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2016 pcbreflux, Apache 2.0 License.
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

#include <driver/gpio.h>

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
#define SMTPS_SERVER "smtp.gmail.com"
#define SMTPS_PORT "465"
#define SMTPS_MAIL_FROM "from@gmail.com"
#define SMTPS_MAIL_RCPT "help1@gmail.com"
#define SMTPS_MAIL_RCPT2 "help2@gmail.com"
#define SMTPS_USER "user@gmail.com"
#define SMTPS_PWD "password"

#define MAIN_POWER 25
#define PIN_LED1 26
#define PIN_LED2 27
#define GPIO_OUTPUT_PIN_MASK  ((1<<MAIN_POWER) | (1<<PIN_LED1) | (1<<PIN_LED2))

#ifndef HIGH
    #define HIGH 1
#endif
#ifndef LOW
    #define LOW 0
#endif

static const char *TAG = "esp32";

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

static void initialise_gpio(void) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE; //disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT; //set as output mode
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_MASK; //bit mask of the pins
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; //disable pull-down mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;   //disable pull-up mode
    gpio_config(&io_conf); //configure GPIO with the given settings

    gpio_set_level(MAIN_POWER, HIGH); // MAIN_POWER
    gpio_set_level(PIN_LED1, LOW); // PIN_LED1
    gpio_set_level(PIN_LED2, LOW); // PIN_LED2
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

static int do_handshake( mbedtls_ssl_context *ssl )
{
    int ret;
    uint32_t flags;
    unsigned char buf[1024];
    memset(buf, 0, 1024);

    /*
     * 4. Handshake
     */
    ESP_LOGI(TAG,"  . Performing the SSL/TLS handshake..." );

    while( ( ret = mbedtls_ssl_handshake( ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
        	ESP_LOGE(TAG," failed\n  ! mbedtls_ssl_handshake returned %d: %s\n\n", ret, buf );
            return( -1 );
        }
    }

    ESP_LOGI(TAG, " ok\n    [ Ciphersuite is %s ]\n",
            mbedtls_ssl_get_ciphersuite( ssl ) );

    /*
     * 5. Verify the server certificate
     */
    ESP_LOGI(TAG, "  . Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        ESP_LOGE(TAG, " failed\n" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        ESP_LOGE(TAG, "%s\n", vrfy_buf );
    } else {
    	ESP_LOGI(TAG, " ok\n" );
    }

    ESP_LOGI(TAG, "  . Peer certificate information    ...\n" );
    mbedtls_x509_crt_info( (char *) buf, sizeof( buf ) - 1, "      ",
                   mbedtls_ssl_get_peer_cert( ssl ) );
    ESP_LOGI(TAG, "%s\n", buf );

    return( 0 );
}

static int write_ssl_data( mbedtls_ssl_context *ssl, unsigned char *buf, size_t len )
{
    int ret;

    ESP_LOGV(TAG,"mbedtls_ssl_write [%s]", buf);
    while( len && ( ret = mbedtls_ssl_write( ssl, buf, len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
        	ESP_LOGE(TAG, " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            return -1;
        }
    }

    return( 0 );
}

static int write_ssl_and_get_response( mbedtls_ssl_context *ssl, unsigned char *buf, size_t len )
{
    int ret;
    unsigned char data[128];
    char code[4];
    size_t i, idx = 0;

    ESP_LOGI(TAG,"mbedtls_ssl_write [%s]", buf);
    while( len && ( ret = mbedtls_ssl_write( ssl, buf, len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
        	ESP_LOGE(TAG, " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            return -1;
        }
    }

    do
    {
        len = sizeof( data ) - 1;
        memset( data, 0, sizeof( data ) );
        ret = mbedtls_ssl_read( ssl, data, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            return -1;

        if( ret <= 0 )
        {
        	ESP_LOGE(TAG, "failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
            return -1;
        }

        ESP_LOGI(TAG,"mbedtls_ssl_read [%s]\n", data);
        len = ret;
        for( i = 0; i < len; i++ )
        {
            if( data[i] != '\n' )
            {
                if( idx < 4 )
                    code[ idx++ ] = data[i];
                continue;
            }

            if( idx == 4 && code[0] >= '0' && code[0] <= '9' && code[3] == ' ' )
            {
                code[3] = '\0';
                return atoi( code );
            }

            idx = 0;
        }
    }
    while( 1 );
}

static void smtps_task(void *pvParameters)
{
    char buf[512];
	unsigned char msgbuf[512];
    unsigned char base[1024];
    int ret, flags, len;
    size_t n;

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
    if((ret = mbedtls_ssl_set_hostname(&ssl, SMTPS_SERVER)) != 0)
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

        ESP_LOGI(TAG, "Connecting to %s:%s...", SMTPS_SERVER, SMTPS_PORT);

        if ((ret = mbedtls_net_connect(&server_fd, SMTPS_SERVER,
        		SMTPS_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
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

        if( do_handshake( &ssl ) != 0 )
            goto exit;

        ESP_LOGI(TAG, "  > Get header from server:" );
        fflush( stdout );

        ret = write_ssl_and_get_response( &ssl, msgbuf, 0 );
        if( ret < 200 || ret > 299 )
        {
        	ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
            goto exit;
        }

        ESP_LOGI(TAG," ok" );

        ESP_LOGI(TAG, "  > Write EHLO to server:" );
        fflush( stdout );

        len = sprintf( (char *) msgbuf, "EHLO ESP32\r\n");
        ret = write_ssl_and_get_response( &ssl, msgbuf, len );
        if( ret < 200 || ret > 299 )
        {
        	ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
            goto exit;
        }

	    ESP_LOGI(TAG, "  > Write AUTH LOGIN to server:" );

		len = sprintf( (char *) msgbuf, "AUTH LOGIN\r\n" );
		ret = write_ssl_and_get_response( &ssl, msgbuf, len );
		if( ret < 200 || ret > 399 )
		{
			ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
		    goto exit;
		}

		ESP_LOGI(TAG, " ok" );

		ESP_LOGI(TAG, "  > Write username to server");
		ESP_LOGV(TAG, "%s", SMTPS_USER );

		ret = mbedtls_base64_encode( base, sizeof( base ), &n, (const unsigned char *) SMTPS_USER,
			             strlen( SMTPS_USER ) );

		if( ret != 0 ) {
			ESP_LOGE(TAG, " failed\n  ! mbedtls_base64_encode returned %d\n\n", ret );
		    goto exit;
		}
		len = sprintf( (char *) msgbuf, "%s\r\n", base );
		ESP_LOGI(TAG, "  > Write base64 username to server: %s", msgbuf );
		ret = write_ssl_and_get_response( &ssl, msgbuf, len );
		if( ret < 300 || ret > 399 )
		{
			ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
		    goto exit;
		}

		ESP_LOGI(TAG," ok" );

		ESP_LOGI(TAG, "  > Write password to server");
		ESP_LOGV(TAG, "%s", SMTPS_PWD );

		ret = mbedtls_base64_encode( base, sizeof( base ), &n, (const unsigned char *) SMTPS_PWD,
			             strlen( SMTPS_PWD ) );

		if( ret != 0 ) {
			ESP_LOGE(TAG, " failed\n  ! mbedtls_base64_encode returned %d\n\n", ret );
		    goto exit;
		}
		len = sprintf( (char *) msgbuf, "%s\r\n", base );
		ESP_LOGI(TAG, "  > Write base64 password to server: %s", msgbuf );
		ret = write_ssl_and_get_response( &ssl, msgbuf, len );
		if( ret < 200 || ret > 399 )
		{
			ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
		    goto exit;
		}

		ESP_LOGI(TAG," ok" );

		ESP_LOGI(TAG, "  > Write MAIL FROM to server:" );

	    len = sprintf( (char *) msgbuf, "MAIL FROM:<%s>\r\n", SMTPS_MAIL_FROM);
	    ret = write_ssl_and_get_response( &ssl, msgbuf, len );
	    if( ret < 200 || ret > 299 )
	    {
	    	ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
	    	goto exit;
	    }

	    ESP_LOGI(TAG," ok" );

	    ESP_LOGI(TAG, "  > Write RCPT TO to server:" );

	    len = sprintf( (char *) msgbuf, "RCPT TO:<%s>\r\n", SMTPS_MAIL_RCPT );
	    ret = write_ssl_and_get_response( &ssl, msgbuf, len );
	    if( ret < 200 || ret > 299 )
	    {
	    	ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
	    	goto exit;
	    }

	    ESP_LOGI(TAG," ok" );

#ifdef SMTPS_MAIL_RCPT2
	    ESP_LOGI(TAG, "  > Write RCPT TO to server:" );

	    len = sprintf( (char *) msgbuf, "RCPT TO:<%s>\r\n", SMTPS_MAIL_RCPT2 );
	    ret = write_ssl_and_get_response( &ssl, msgbuf, len );
	    if( ret < 200 || ret > 299 )
	    {
	    	ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
	    	goto exit;
	    }

	    ESP_LOGI(TAG," ok" );

#endif

	    ESP_LOGI(TAG, "  > Write DATA to server:" );

	    len = sprintf( (char *) msgbuf, "DATA\r\n" );
	    ret = write_ssl_and_get_response( &ssl, msgbuf, len );
	    if( ret < 300 || ret > 399 )
	    {
	    	ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
	    	goto exit;
	    }

	    ESP_LOGI(TAG," ok" );

	    ESP_LOGI(TAG, "  > Write content to server:" );

	    len = sprintf( (char *) msgbuf, "From: %s\r\nSubject: Intruder Detected!\r\n\r\n"
		    "This could be the Text you send "
		    "for the ESP32 TLS mail client example.\r\n"
		    "\r\n"
		    "Sample DIY Radar Intrusion Detector", SMTPS_MAIL_FROM );
	    ret = write_ssl_data( &ssl, msgbuf, len );

	    len = sprintf( (char *) msgbuf, "\r\n.\r\n");
	    ret = write_ssl_and_get_response( &ssl, msgbuf, len );
	    if( ret < 200 || ret > 299 )
	    {
	    	ESP_LOGE(TAG, " failed\n  ! server responded with %d\n\n", ret );
	    	goto exit;
	    }
        ret=0;
	    ESP_LOGI(TAG," ok" );

        mbedtls_ssl_close_notify(&ssl);

    exit:
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);

        if(ret != 0) {
        	gpio_set_level(PIN_LED2, HIGH); // PIN_LED2

            mbedtls_strerror(ret, buf, 100);
            ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
        } else {
        	gpio_set_level(PIN_LED1, HIGH); // PIN_LED1
        }

        for(int countdown = 600; countdown >= 0; countdown--) {
        	if(countdown%10==0) {
        		ESP_LOGI(TAG, "%d...", countdown);
        	}
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
        gpio_set_level(MAIN_POWER, LOW); // MAIN_POWER
        vTaskDelay(1000 / portTICK_RATE_MS);
        ESP_LOGI(TAG, "Brown out, not starting again!");
    }
}

void app_main()
{
    nvs_flash_init();
    //system_init();
    initialise_gpio();
    initialise_wifi();
    xTaskCreate(&smtps_task, "smtps_task", 8192, NULL, 5, NULL);
}
