/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    pcbreflux - mbedtls implementation via websocket
 *******************************************************************************/

#include "MQTTmbedtls.h"

#ifndef timeradd
#define timeradd(a, b, result)                                 \
{                                                              \
  (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                \
  (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;             \
  if ( (result)->tv_usec >= 1000000 ) {                        \
          (result)->tv_sec++; (result)->tv_usec -= 1000000ul;  \
  }                                                            \
}
#endif

#ifndef timersub
#define timersub(a, b, result)                                 \
{                                                              \
  (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                \
  (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;             \
  if ( (result)->tv_usec < 0 ) {                               \
          (result)->tv_sec--; (result)->tv_usec += 1000000ul;  \
  }                                                            \
}
#endif

static const char *TAG = "MQTTmbedtls";

/* Root cert for mqtt.thingspeak.com, found in cert.c */
extern const char *server_root_cert;

void TimerInit(Timer* timer)
{
	timer->end_time = (struct timeval){0, 0};
}

char TimerIsExpired(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	timeradd(&now, &interval, &timer->end_time);
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout, 0};
	timeradd(&now, &interval, &timer->end_time);
}


int TimerLeftMS(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

int websocket_create_frame(uint8_t opcode,unsigned char* framebuffer, unsigned char* data,int len) {

	int bytepos = 0;
	uint8_t mask_flag = 1;
    int mask_key[4] =   { (int)(esp_random()&0xFF),(int)(esp_random()&0xFF),(int)(esp_random()&0xFF),(int)(esp_random()&0xFF) };
   // mask_flag = do_masking

	framebuffer[bytepos] = (1 << 7 | opcode);
	bytepos++;
    if (len < 126) {
    	framebuffer[bytepos] = (mask_flag << 7 | len);
    	bytepos++;
    } else if (len < 32768) {
    	framebuffer[bytepos] = (mask_flag << 7 | 126);
    	bytepos++;
    	framebuffer[bytepos] = (len >> 8  & 0xFF);
    	bytepos++;
    	framebuffer[bytepos] = (len & 0xFF);
    	bytepos++;
    } // ToDo : Frame >= 32768 i.e. len == 127
	if (mask_flag == 1) {
    	framebuffer[bytepos] = mask_key[0];
    	bytepos++;
    	framebuffer[bytepos] = mask_key[1];
    	bytepos++;
    	framebuffer[bytepos] = mask_key[2];
    	bytepos++;
    	framebuffer[bytepos] = mask_key[3];
    	bytepos++;
	}
	for (int i=0;i<len;i++) {
		if (mask_flag == 1) {
			framebuffer[bytepos+i]=data[i]^mask_key[i%4];  // Bitwise XOR at index modulo 4
		} else {
			framebuffer[bytepos+i]=data[i];
		}
	}
    return bytepos+len;
}

int websocket_remove_frame(Network* n,unsigned char* framebuffer, unsigned char* data, int framlen) {

	int bytepos = 0;
	int opcode;
	uint8_t mask_flag = 1;
    int mask_key[4] =   { 0x00,0x00,0x00,0x00 };
    int len;
   // mask_flag = do_masking

    opcode = framebuffer[bytepos] & 0x0F;
	bytepos++;
	len = framebuffer[bytepos] & 0x7F;
	mask_flag = framebuffer[bytepos]>>7;
	bytepos++;
    if (len == 126) {
    	len = framebuffer[bytepos]<<8;
    	bytepos++;
    	len += framebuffer[bytepos];
    	bytepos++;
    }
    if (len == 127) {
    	len = 0; // ToDo : Frame >= 32768 i.e. len == 127
        goto exit;
    }

    if (len>framlen) {
    	len = framlen;
    }
	if (mask_flag == 1) {
		mask_key[0] = framebuffer[bytepos];
    	bytepos++;
    	mask_key[1] = framebuffer[bytepos];
    	bytepos++;
    	mask_key[2] = framebuffer[bytepos];
    	bytepos++;
    	mask_key[3] = framebuffer[bytepos];
    	bytepos++;
	}
	for (int i=0;i<len;i++) {
		if (mask_flag == 1) {
			data[i]=framebuffer[bytepos+i]^mask_key[i%4];  // Bitwise XOR at index modulo 4
		} else {
			data[i]=framebuffer[bytepos+i];
		}
	}
	if (opcode==WEBSOCKET_CONNCLOSE||opcode==WEBSOCKET_PING) {
		if (opcode==WEBSOCKET_PING) {
			opcode=WEBSOCKET_PONG;
		}
		ESP_LOGI(TAG, "websocket_mbedtls_write mqtt want %d",len);
		int framelen = websocket_create_frame(opcode,n->ws_sendbuf,data,len);

		for (int i=0;i<framelen;i++) {
			ESP_LOGI(TAG, "websocket_mbedtls_write: %d %02X [%c]",i+1, n->ws_sendbuf[i], n->ws_sendbuf[i]);
		}
		int ret = mbedtls_ssl_write(&n->ssl, n->ws_sendbuf, framelen);
		ESP_LOGI(TAG, "mbedtls_ssl_write websocket %d from %d",ret,len);
	}

exit:
    return len;
}

int websocket_mbedtls_read(Network* n) {
	n->mqtt_recv_len=0;
	n->mqtt_recv_offset=0;
	do {
		int ret = mbedtls_ssl_read(&n->ssl, n->ws_recvbuf,MBEDTLS_WEBSOCKET_RECV_BUF_LEN);
		if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
			continue;
		}
		if(ret <= 0) {
			ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
			ret = 0;
			break;
		}
		n->ws_recv_len=ret;
	    ESP_LOGI(TAG, "mbedtls_ssl_read get ws len %d",n->ws_recv_len);
		for (int i=0;i<n->ws_recv_len;i++) {
			ESP_LOGD(TAG, "3 mbedtls_ssl_read: %d %02X [%c]",i+1, n->ws_recvbuf[i], n->ws_recvbuf[i]);
		}
		n->mqtt_recv_len=websocket_remove_frame(n,n->ws_recvbuf,n->mqtt_recvbuf,n->ws_recv_len);
	    ESP_LOGI(TAG, "mbedtls_ssl_read get mqtt len %d",n->mqtt_recv_len);
		break;
	} while (1);

	return n->mqtt_recv_len;
}

int mqtt_mbedtls_read(Network* n, unsigned char* buffer, int len, int timeout_ms) {

	// ToDo: timeout_ms handling
	int bytes = 0;

    ESP_LOGI(TAG, "mbedtls_ssl_read want len %d",len);
	if (n->ws_recv_len==0) {
		bytes=websocket_mbedtls_read(n);
		if (bytes==0) {
			goto exit;
		}
	}
	if (len <= (n->mqtt_recv_len - n->mqtt_recv_offset)) { // buffer full
		for (int i=0;i<len;i++) {
			buffer[i]=n->mqtt_recvbuf[n->mqtt_recv_offset];
			n->mqtt_recv_offset++;
		}
		bytes=len;
	} else {  // buffer near empty
		int buffoffset=0;
		while (n->mqtt_recv_offset<=n->mqtt_recv_len) { // write remain
			buffer[buffoffset]=n->mqtt_recvbuf[n->mqtt_recv_offset];
			n->mqtt_recv_offset++;
			buffoffset++;
		}
		while (buffoffset<len) {
			while (n->mqtt_recv_offset<=n->mqtt_recv_len && buffoffset<len) { // write remain
				buffer[buffoffset]=n->mqtt_recvbuf[n->mqtt_recv_offset];
				n->mqtt_recv_offset++;
				buffoffset++;
			}
			if (buffoffset<len) {
				bytes=websocket_mbedtls_read(n);
				if (bytes==0) {
					goto exit;
				}
			}
		}
		bytes=len;
	}
	if (n->mqtt_recv_offset==n->mqtt_recv_len) { // buffer empty
		n->mqtt_recv_len=0;
		n->mqtt_recv_offset=0;
	}
exit:
	ESP_LOGI(TAG, "mqtt_mbedtls_read get %d %d %d",bytes,n->mqtt_recv_offset,n->mqtt_recv_len);

	return bytes;
}

int websocket_mbedtls_write(Network* n, unsigned char* buffer, int len) {

	ESP_LOGI(TAG, "websocket_mbedtls_write mqtt want %d",len);
    for (int i=0;i<len;i++) {
    	ESP_LOGD(TAG, "websocket_mbedtls_write: %d %02X [%c]",i+1, buffer[i], buffer[i]);
    }
	int framelen = websocket_create_frame(WEBSOCKET_BINARY,n->ws_sendbuf,buffer,len);

    for (int i=0;i<framelen;i++) {
    	ESP_LOGD(TAG, "websocket_mbedtls_write: %d %02X [%c]",i+1, n->ws_sendbuf[i], n->ws_sendbuf[i]);
    }
    int ret = mbedtls_ssl_write(&n->ssl, n->ws_sendbuf, framelen);
    ESP_LOGI(TAG, "mbedtls_ssl_write websocket %d from %d",ret,framelen);

	return ret;
}

int mqtt_mbedtls_write(Network* n, unsigned char* buffer, int len, int timeout_ms) {

	// ToDo: timeout_ms handling
    ESP_LOGI(TAG, "mqtt_mbedtls_write len %d",len);
    int ret = websocket_mbedtls_write(n, buffer, len);
    if (ret>=len) {  // websocket frame around mqtt
    	ret = len;
    }
    return ret;
}


void NetworkInit(Network* n)
{
	n->ws_recv_offset=0;
	n->ws_recv_len=0;
	n->mqtt_recv_offset=0;
	n->mqtt_recv_len=0;
	n->mqttread = mqtt_mbedtls_read;
	n->mqttwrite = mqtt_mbedtls_write;
}


int NetworkConnect(Network* n, char* addr, int port)
{
    char portbuf[100];
    int ret, flags;
	int retVal = -1;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;

    mbedtls_ssl_init(&n->ssl);
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
    if((ret = mbedtls_ssl_set_hostname(&n->ssl, addr)) != 0)
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

    if ((ret = mbedtls_ssl_setup(&n->ssl, &conf)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

	mbedtls_net_init(&n->server_fd);

	sprintf(portbuf,"%d",port);
	ESP_LOGI(TAG, "Connecting to %s:%s...", addr, portbuf);

	if ((ret = mbedtls_net_connect(&n->server_fd, addr,
								  portbuf, MBEDTLS_NET_PROTO_TCP)) != 0)
	{
		ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
		goto exit;
	}

	ESP_LOGI(TAG, "Connected.");

	mbedtls_ssl_set_bio(&n->ssl, &n->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

	ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");

	while ((ret = mbedtls_ssl_handshake(&n->ssl)) != 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
			goto exit;
		}
	}

	ESP_LOGI(TAG, "Verifying peer X.509 certificate...");

	if ((flags = mbedtls_ssl_get_verify_result(&n->ssl)) != 0)
	{
		/* In real life, we probably want to close connection if ret != 0 */
		ESP_LOGW(TAG, "Failed to verify peer certificate!");
		bzero(n->ws_recvbuf, sizeof(n->ws_recvbuf));
		mbedtls_x509_crt_verify_info((char *)n->ws_recvbuf, sizeof(n->ws_recvbuf), "  ! ", flags);
		ESP_LOGW(TAG, "verification info: %s", n->ws_recvbuf);
	}
	else {
		ESP_LOGI(TAG, "Certificate verified.");
	}
	int len;

        // ToDo: generate random UUID i.e. Sec-WebSocket-Key like unix command: uuidgen -r | base64 -
	sprintf((char *)n->ws_sendbuf,"GET /mqtt HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nHost: %s:%d\r\nOrigin: https://%s:%d\r\nSec-WebSocket-Key: NDQzNjNmOTAtNzQ5ZC00MDVjLTg0N2QtM2ZiMjRhYTM1ZmY3Cg==\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: mqtt\r\n\r\n",addr,port,addr,port);

	ESP_LOGI(TAG, "req=[%s]",n->ws_sendbuf);

	while((ret = mbedtls_ssl_write(&n->ssl,n->ws_sendbuf,strlen((const char *)n->ws_sendbuf))) <= 0) {
		if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
			goto exit;
		}
	}

	len = ret;
	ESP_LOGI(TAG, "%d bytes written", len);
	ESP_LOGI(TAG, "Reading HTTP response...");

	do {
		len = sizeof(n->ws_recvbuf) - 1;
		bzero(n->ws_recvbuf, sizeof(n->ws_recvbuf));
		ret = mbedtls_ssl_read(&n->ssl, n->ws_recvbuf, len);

		if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
			continue;
		}

		if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
			ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
			ret = 0;
			break;
		}

		if(ret < 0) {
			ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
			break;
		}

		if(ret == 0) {
			ESP_LOGI(TAG, "connection closed");
			break;
		}

		len = ret;
		ESP_LOGI(TAG, "%d bytes read", len);
		/* Print response directly to stdout as it is read */
		for(int i = 0; i < len; i++) {
			putchar(n->ws_recvbuf[i]);
		}
		break;
	} while(1);

    // "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" // ToDo: realy check answer for security and man in the middle attacks
    if (strncmp((const char *)n->ws_recvbuf, "HTTP/1.1 101 Switching Protocols",32)!=0) {
		goto exit;
    }

	retVal = 0;

	return retVal;

exit:
        mbedtls_ssl_session_reset(&n->ssl);
        mbedtls_net_free(&n->server_fd);

	return retVal;
}


void NetworkDisconnect(Network* n)
{
    mbedtls_ssl_session_reset(&n->ssl);
    mbedtls_net_free(&n->server_fd);
    n->ws_recv_offset=0;
    n->ws_recv_len=0;
    n->mqtt_recv_offset=0;
    n->mqtt_recv_len=0;
}

