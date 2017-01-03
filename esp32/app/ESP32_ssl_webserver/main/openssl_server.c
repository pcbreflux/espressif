/* OpenSSL server Example
 *
 * Original Copyright (C) 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 * Additions Copyright (C) 2016 pcbreflux, Apache 2.0 License.
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

#include "openssl_server.h"

#include <string.h>

#include "openssl/ssl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "driver/gpio.h"

static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const static int CONNECTED_BIT = BIT0;

const static char *TAG = "Openssl_demo";

static uint8_t led_pin[] = { 5,0,13,0,18,0,19,0 };
static int led_cnt = sizeof(led_pin)/sizeof(led_pin[0]);

/* returns number of strings replaced.
*/
int replacestr(char *line, const char *search, const char *replace)
{
   int count;
   char *sp; // start of pattern

   //printf("replacestr(%s, %s, %s)\n", line, search, replace);
   if ((sp = strstr(line, search)) == NULL) {
      return(0);
   }
   count = 1;
   int sLen = strlen(search);
   int rLen = strlen(replace);
   if (sLen > rLen) {
      // move from right to left
      char *src = sp + sLen;
      char *dst = sp + rLen;
      while((*dst = *src) != '\0') { dst++; src++; }
   } else if (sLen < rLen) {
      // move from left to right
      int tLen = strlen(sp) - sLen;
      char *stop = sp + rLen;
      char *src = sp + sLen + tLen;
      char *dst = sp + rLen + tLen;
      while(dst >= stop) { *dst = *src; dst--; src--; }
   }
   memcpy(sp, replace, rLen);

   count += replacestr(sp + rLen, search, replace);

   return(count);
}

void openssl_demo_write(SSL *ssl,const char *msg, const char *search, const char *replace) {
	char send_buf[OPENSSL_DEMO_SEND_BUF_LEN];

    ESP_LOGD(TAG, "memcpy %s %d", msg,strlen(msg));
	memcpy(send_buf,msg,strlen(msg)+1);

	if (search && replace) {
	    ESP_LOGD(TAG, "send_buf %s %d", send_buf,strlen(send_buf));
	    ESP_LOGD(TAG, "search %s", search);
	    ESP_LOGD(TAG, "replace %s", replace);
		replacestr(send_buf,search,replace);
	}
    ESP_LOGD(TAG, "send_buf %s %d", send_buf,strlen(send_buf));
	SSL_write(ssl, send_buf, strlen(send_buf));
}

void openssl_demo_write3(SSL *ssl,const char *msg, const char *search, const char *replace, const char *search2, const char *replace2, const char *search3, const char *replace3) {
	char send_buf[OPENSSL_DEMO_SEND_BUF_LEN];

    ESP_LOGI(TAG, "memcpy %s %d", msg,strlen(msg));
	memcpy(send_buf,msg,strlen(msg)+1);

	if (search && replace) {
	    ESP_LOGD(TAG, "send_buf %s %d", send_buf,strlen(send_buf));
	    ESP_LOGD(TAG, "search %s", search);
	    ESP_LOGD(TAG, "replace %s", replace);
		replacestr(send_buf,search,replace);
	}
	if (search2 && replace2) {
	    ESP_LOGD(TAG, "send_buf %s %d", send_buf,strlen(send_buf));
	    ESP_LOGD(TAG, "search %s", search);
	    ESP_LOGD(TAG, "replace %s", replace);
		replacestr(send_buf,search2,replace2);
	}
	if (search3 && replace3) {
	    ESP_LOGD(TAG, "send_buf %s %d", send_buf,strlen(send_buf));
	    ESP_LOGD(TAG, "search %s", search);
	    ESP_LOGD(TAG, "replace %s", replace);
		replacestr(send_buf,search3,replace3);
	}
    ESP_LOGD(TAG, "send_buf %s %d", send_buf,strlen(send_buf));
	SSL_write(ssl, send_buf, strlen(send_buf));
}

void write_nvs_led_pin(uint8_t pin, uint8_t value) {
	nvs_handle my_handle;
	esp_err_t err;
    char value_key[20];

	// Open
	ESP_LOGI(TAG, "NVS Opening Non-Volatile Storage ... ");
	err = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		ESP_LOGI(TAG, "NVS Error (%d) opening!", err);
	} else {
		ESP_LOGI(TAG, "Done\n");

    	sprintf(value_key,"GPIO%02d",pin);
		// Write
    	ESP_LOGI(TAG, "NVS Updating %s value %d ... ",value_key, value);
		err = nvs_set_u8(my_handle, value_key, value);
		if (err != ESP_OK) {
			ESP_LOGI(TAG, "NVS Failed!");
		} else {
			ESP_LOGI(TAG, "NVS Done");
		}

		// Commit written value.
		// After setting any values, nvs_commit() must be called to ensure changes are written
		// to flash storage. Implementations may write to storage at other times,
		// but this is not guaranteed.
		ESP_LOGI(TAG, "NVS Committing updates ... ");
		err = nvs_commit(my_handle);
		if (err != ESP_OK) {
			ESP_LOGI(TAG, "NVS Failed!");
		} else {
			ESP_LOGI(TAG, "NVS Done");
		}

		// Close
		nvs_close(my_handle);
	}
}

uint8_t read_nvs_led_pin(uint8_t pin) {
	nvs_handle my_handle;
	esp_err_t err;
    char value_key[20];
    uint8_t value=0;

	// Open
	ESP_LOGI(TAG, "NVS Opening Non-Volatile Storage ... ");
	err = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		ESP_LOGI(TAG, "NVS Error (%d) opening!", err);
	} else {
		ESP_LOGI(TAG, "Done");

    	sprintf(value_key,"GPIO%02d",pin);
		// Read
    	ESP_LOGI(TAG, "NVS Reading pins ... ");
		err = nvs_get_u8(my_handle, value_key, &value);
		switch (err) {
			case ESP_OK:
				ESP_LOGI(TAG, "NVS value %s = %d",value_key, value);
				break;
			case ESP_ERR_NVS_NOT_FOUND:
				printf("NVS The value %s is not initialized yet!",value_key);
				value=0;
				break;
			default :
				printf("NVS Error (%d) reading!", err);
		}
		// Close
		nvs_close(my_handle);
	}

	return value;
}

static void openssl_demo_thread(void *p)
{
    int ret;

    SSL_CTX *ctx;
    SSL *ssl;

    int socket, new_socket;
    socklen_t addr_len;
    struct sockaddr_in sock_addr;

    char recv_buf[OPENSSL_DEMO_RECV_BUF_LEN];

    extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
    extern const unsigned char cacert_pem_end[]   asm("_binary_cacert_pem_end");
    const unsigned int cacert_pem_bytes = cacert_pem_end - cacert_pem_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
    const unsigned int prvtkey_pem_bytes = prvtkey_pem_end - prvtkey_pem_start;   

    extern const unsigned char logo_png_start[] asm("_binary_logo_png_start");
    extern const unsigned char logo_png_end[]   asm("_binary_logo_png_end");
    const unsigned int logo_png_bytes = logo_png_end - logo_png_start;

    ESP_LOGI(TAG, "SSL server context create ......");
    ctx = SSL_CTX_new(TLSv1_2_server_method());
    if (!ctx) {
        ESP_LOGI(TAG, "failed");
        goto failed1;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server context set own certification......");
    ret = SSL_CTX_use_certificate_ASN1(ctx, cacert_pem_bytes, cacert_pem_start);
    if (!ret) {
        ESP_LOGI(TAG, "failed");
        goto failed2;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server context set private key......");
    ret = SSL_CTX_use_PrivateKey_ASN1(0, ctx, prvtkey_pem_start, prvtkey_pem_bytes);
    if (!ret) {
        ESP_LOGI(TAG, "failed");
        goto failed2;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server create socket ......");
    socket = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        ESP_LOGI(TAG, "failed");
        goto failed2;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server socket bind ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = 0;
    sock_addr.sin_port = htons(OPENSSL_DEMO_LOCAL_TCP_PORT);
    ret = bind(socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (ret) {
        ESP_LOGI(TAG, "failed");
        goto failed3;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server socket listen ......");
    ret = listen(socket, 32);
    if (ret) {
        ESP_LOGI(TAG, "failed");
        goto failed3;
    }
    ESP_LOGI(TAG, "OK");

reconnect:
    ESP_LOGI(TAG, "SSL server create ......");
    ssl = SSL_new(ctx);
    if (!ssl) {
        ESP_LOGI(TAG, "failed");
        goto failed3;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server socket accept client ......");
    new_socket = accept(socket, (struct sockaddr *)&sock_addr, &addr_len);
    if (new_socket < 0) {
        ESP_LOGI(TAG, "failed" );
        goto failed4;
    }
    ESP_LOGI(TAG, "OK");

    SSL_set_fd(ssl, new_socket);

    ESP_LOGI(TAG, "SSL server accept client ......");
    ret = SSL_accept(ssl);
    if (!ret) {
        ESP_LOGI(TAG, "failed");
        goto failed5;
    }
    ESP_LOGI(TAG, "OK");

    ESP_LOGI(TAG, "SSL server read message ......");
    char form_text[127];
    char button_text[20];

    do {
        memset(recv_buf, 0, OPENSSL_DEMO_RECV_BUF_LEN);
        ret = SSL_read(ssl, recv_buf, OPENSSL_DEMO_RECV_BUF_LEN - 1);
        ESP_LOGI(TAG, "recv_buf %s", recv_buf);
        if (ret <= 0) {
            break;
        }
        int openssl_demo_main=0;

        for (int i=0;i<led_cnt;i+=2) {
        	sprintf(form_text,"GET /GPIO%d HTTP/1.1",led_pin[i]);
            if (strncmp(recv_buf, form_text,strlen(form_text))==0) {
                led_pin[i+1]=1-led_pin[i+1];
                write_nvs_led_pin(led_pin[i], led_pin[i+1]);
                gpio_set_level(led_pin[i], led_pin[i+1]);
            	openssl_demo_main=1;
            }
        }
        if (strncmp(recv_buf, "GET / HTTP/1.1",14)==0) {
        	openssl_demo_main=1;
        }
        if (openssl_demo_main==1) {
        	openssl_demo_write(ssl, OPENSSL_DEMO_HTTP_200_TXT,NULL,NULL);
        	openssl_demo_write(ssl, OPENSSL_DEMO_HTTP_HEAD,"{v}","ESP32 SSL Demo");
        	openssl_demo_write(ssl, OPENSSL_DEMO_HTTP_SCRIPT,NULL,NULL);
        	openssl_demo_write(ssl, OPENSSL_DEMO_HTTP_STYLE,NULL,NULL);
        	openssl_demo_write(ssl, OPENSSL_DEMO_HTTP_HEAD_END,NULL,NULL);
        	openssl_demo_write(ssl, OPENSSL_DEMO_HTTP_LOGO,NULL,NULL);
        	openssl_demo_write(ssl, "&nbsp;ESP32 OpenSSL GPIO Server Demo!<BR><BR>\r\n",NULL,NULL);

            for (int i=0;i<led_cnt;i+=2) {
            	sprintf(form_text,"GPIO%d",led_pin[i]);
            	sprintf(button_text,"GPIO %d %s",led_pin[i],led_pin[i+1]==0?"on":"off");
        		openssl_demo_write3(ssl, led_pin[i+1]==0?OPENSSL_DEMO_HTTP_FORM_BUTTON_ON:OPENSSL_DEMO_HTTP_FORM_BUTTON_OFF,"{f}",form_text,"{l}",button_text,NULL,NULL);
        	}
        	openssl_demo_write(ssl, OPENSSL_DEMO_HTTP_END,NULL,NULL);
            break;
        }
        if (strncmp(recv_buf, "GET /logo.png HTTP/1.1",22)==0) {
        	openssl_demo_write(ssl, OPENSSL_DEMO_HTTP_200_PNG,NULL,NULL);
            int offset=0;
            int send_bytes=OPENSSL_DEMO_LOCAL_TCP_MTU;
            while (offset<logo_png_bytes) {
            	if (offset+send_bytes>logo_png_bytes) {
            		send_bytes=logo_png_bytes-offset;
            	}
            	SSL_write(ssl, &logo_png_start[offset], send_bytes);
            	offset+=send_bytes;
            }

            break;
        }
    } while (1);
    
    ESP_LOGI(TAG, "result %d", ret);

    SSL_shutdown(ssl);
failed5:
    close(new_socket);
    new_socket = -1;
failed4:
    SSL_free(ssl);
    ssl = NULL;
    goto reconnect;
failed3:
    close(socket);
    socket = -1;
failed2:
    SSL_CTX_free(ctx);
    ctx = NULL;
failed1:
    vTaskDelete(NULL);
    return ;
} 

static void openssl_client_init(void)
{
    int ret;
    xTaskHandle openssl_handle;

    ret = xTaskCreate(openssl_demo_thread,
                      OPENSSL_DEMO_THREAD_NAME,
                      OPENSSL_DEMO_THREAD_STACK_WORDS,
                      NULL,
                      OPENSSL_DEMO_THREAD_PRORIOTY,
                      &openssl_handle); 

    if (ret != pdPASS)  {
        ESP_LOGI(TAG, "create thread %s failed", OPENSSL_DEMO_THREAD_NAME);
    }
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        openssl_client_init();
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

static void openssl_gpio_init(void)
{

    for (int i=0;i<led_cnt;i+=2) {
        gpio_pad_select_gpio(led_pin[i]);
        gpio_set_direction(led_pin[i], GPIO_MODE_OUTPUT);
       	led_pin[i+1] = read_nvs_led_pin(led_pin[i]);
        gpio_set_level(led_pin[i], led_pin[i+1]);
    }
}


static void wifi_conn_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]\n", EXAMPLE_WIFI_SSID, EXAMPLE_WIFI_PASS);
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void app_main(void)
{
    nvs_flash_init();
    openssl_gpio_init();
    wifi_conn_init();
}
