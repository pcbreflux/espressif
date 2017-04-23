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

#ifdef MBEDTLS_DEBUG_C

#define MBEDTLS_DEBUG_LEVEL 3

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

static const char *TAG = "MQTTmbedtls";

/* Root cert for mqtt.thingspeak.com, found in cert.c */
extern const char *server_root_cert;

void TimerInit(Timer* timer) {
	ESP_LOGD(TAG, "TimerInit");
	timer->end_time = (struct timeval){0, 0};
}

char TimerIsExpired(Timer* timer) {
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	ESP_LOGD(TAG, "TimerIsExpired %ld.%ld",res.tv_sec,res.tv_usec);
	ESP_LOGD(TAG, "TimerIsExpired %d",res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0));
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout) {
	struct timeval now;
	ESP_LOGD(TAG, "TimerCountdownMS");
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	timeradd(&now, &interval, &timer->end_time);
}


void TimerCountdown(Timer* timer, unsigned int timeout) {
	ESP_LOGD(TAG, "TimerCountdown");
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout, 0};
	timeradd(&now, &interval, &timer->end_time);
}


int TimerLeftMS(Timer* timer) {
	ESP_LOGD(TAG, "TimerLeftMS");
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

int websocket_create_frame(uint8_t opcode,unsigned char* framebuffer, unsigned char* data,int len,uint8_t mask_flag) {

	int bytepos = 0;
#if defined(MBEDTLS_MQTT_DEBUG)
    int mask_key[4] =   { 0x01,0x02,0x03,0x04 };
#else
    int mask_key[4] =   { (int)(esp_random()&0xFF),(int)(esp_random()&0xFF),(int)(esp_random()&0xFF),(int)(esp_random()&0xFF) };
#endif

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
		ESP_LOGD(TAG, "opcode %d websocket_mbedtls_write mqtt want %d",opcode,len);
		int framelen = websocket_create_frame(opcode,n->ws_sendbuf,data,len,0);

		for (int i=0;i<framelen;i++) {
			ESP_LOGD(TAG, "opcode websocket_mbedtls_write: %d %02X [%c]",i+1, n->ws_sendbuf[i], n->ws_sendbuf[i]);
		}
		int ret = mbedtls_ssl_write(&n->ssl, n->ws_sendbuf, framelen);
		ESP_LOGD(TAG, "opcode mbedtls_ssl_write websocket %d from %d",ret,len);
	}

exit:
    return len;
}

int buffered_mbedtls_read(Network* n, int timeout_ms) {
	n->mqtt_recv_len=0;
	n->mqtt_recv_offset=0;
	n->ws_recv_len=0;
	int ret=0;

	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	setsockopt(n->server_fd.fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	do {
	    ESP_LOGD(TAG, "buffered_mbedtls_read (%08X) %d",(unsigned int)&n->ssl,timeout_ms);
		if (n->websocket) {
			bzero(n->ws_recvbuf, sizeof(n->ws_recvbuf));
			ret = mbedtls_ssl_read(&n->ssl, n->ws_recvbuf,MBEDTLS_WEBSOCKET_RECV_BUF_LEN-1);
		} else {
			bzero(n->mqtt_recvbuf, sizeof(n->mqtt_recvbuf));
			ret = mbedtls_ssl_read(&n->ssl, n->mqtt_recvbuf,MBEDTLS_MQTT_RECV_BUF_LEN-1);
		}
		if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
			continue;
		}
		if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || ret == -0x4C) {
			ESP_LOGD(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
			ret = 0;
			break;
		}
		if(ret == 0) {
			ESP_LOGD(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
			ret = -1;
			break;
		}
		if(ret < 0) {
			ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
			ret = -1;
			break;
		}
		if (n->websocket) {
			n->ws_recv_len=ret;
		    ESP_LOGD(TAG, "mbedtls_ssl_read get ws len %d",n->ws_recv_len);
#if defined(MBEDTLS_MQTT_DEBUG)
			for (int i=0;i<n->ws_recv_len;i++) {
		    	printf("0x%02X,",n->ws_recvbuf[i]);
			}
			printf("\n");
#endif
			n->mqtt_recv_len=websocket_remove_frame(n,n->ws_recvbuf,n->mqtt_recvbuf,n->ws_recv_len);
		} else {
			n->mqtt_recv_len=ret;
		}
#if defined(MBEDTLS_MQTT_DEBUG)
		for (int i=0;i<n->mqtt_recv_len;i++) {
			printf("0x%02X,",n->mqtt_recvbuf[i]);
		}
		printf("\n");
#endif
		break;
	} while (1);

    ESP_LOGD(TAG, "mbedtls_ssl_read get mqtt len %d (%d)",n->mqtt_recv_len,ret);
	return ret;
}

int mqtt_mbedtls_read(Network* n, unsigned char* buffer, int len, int timeout_ms) {

	int bytes = 0;

    ESP_LOGD(TAG, "mqtt_mbedtls_read want len %d (%d off %d from %d)",len,n->ws_recv_len,n->mqtt_recv_offset,n->mqtt_recv_len);
	if (n->mqtt_recv_len==0) {
		bytes=buffered_mbedtls_read(n, timeout_ms);
		if (bytes<=0) {
			goto exit;
		}
	}
	if (len <= (n->mqtt_recv_len - n->mqtt_recv_offset)) { // buffer full
		for (int i=0;i<len;i++) {
			buffer[i]=n->mqtt_recvbuf[n->mqtt_recv_offset];
			n->mqtt_recv_offset++;
		}
		bytes=len;
	} else {  // buffer near empty ToDo: not realy testested yet
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
				bytes=buffered_mbedtls_read(n, timeout_ms);
				if (bytes<=0) {
					goto exit;
				}
			}
		}
		bytes=len;
	}
	if (n->mqtt_recv_offset==n->mqtt_recv_len) { // buffer empty
		n->mqtt_recv_len=0;
		n->mqtt_recv_offset=0;
		n->ws_recv_len=0;
	}
exit:
	ESP_LOGD(TAG, "mqtt_mbedtls_read get %d %d %d %d",bytes,n->mqtt_recv_offset,n->mqtt_recv_len,bytes);

	return bytes;
}

int buffered_mbedtls_write(Network* n, unsigned char* buffer, int len, int timeout_ms) {
	int ret=0;

	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}


	ESP_LOGD(TAG, "buffered_mbedtls_write (%08X) mqtt want %d (%d)",(unsigned int)&n->ssl,len,timeout_ms);
#if defined(MBEDTLS_MQTT_DEBUG)
    for (int i=0;i<len;i++) {
    	printf("0x%02X,",buffer[i]);
    }
	printf("\n");
#endif
	setsockopt(n->server_fd.fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&interval, sizeof(struct timeval));

	if (n->websocket) {
		bzero(n->ws_sendbuf, sizeof(n->ws_sendbuf));
		int framelen = websocket_create_frame(WEBSOCKET_BINARY,n->ws_sendbuf,buffer,len,1);
#if defined(MBEDTLS_MQTT_DEBUG)
		for (int i=0;i<framelen;i++) {
			printf("0x%02X,",n->ws_sendbuf[i]);
		}
		printf("\n");
#endif
		ret = mbedtls_ssl_write(&n->ssl, n->ws_sendbuf, framelen);
	    ESP_LOGD(TAG, "mbedtls_ssl_write (%08X) websocket %d from %d",(unsigned int)&n->ssl,ret,framelen);
	} else {
		ret = mbedtls_ssl_write(&n->ssl, buffer, len);
	    ESP_LOGD(TAG, "mbedtls_ssl_write (%08X) %d from %d",(unsigned int)&n->ssl,ret,len);
	}

	return ret;
}

int mqtt_mbedtls_write(Network* n, unsigned char* buffer, int len, int timeout_ms) {

    ESP_LOGD(TAG, "mqtt_mbedtls_write len %d",len);
    int ret = buffered_mbedtls_write(n, buffer, len, timeout_ms);
    if (ret>len) {  // websocket frame around mqtt
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
	n->mqttread=mqtt_mbedtls_read;
	n->mqttwrite=mqtt_mbedtls_write;
	n->websocket=0;
	n->connect=0;
	n->init=0;
}


int NetworkConnect(Network* n, char* addr, int port) {
    char portbuf[100];
    int ret, flags;
	int retVal = -1;
	int len;

    mbedtls_ssl_init(&n->ssl);
    mbedtls_x509_crt_init(&n->cacert);
    mbedtls_ctr_drbg_init(&n->ctr_drbg);
    ESP_LOGD(TAG, "Seeding the random number generator");
	n->init=1;

    mbedtls_ssl_config_init(&n->conf);

    mbedtls_entropy_init(&n->entropy);
    if((ret = mbedtls_ctr_drbg_seed(&n->ctr_drbg, mbedtls_entropy_func, &n->entropy,
                                    NULL, 0)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }


    ESP_LOGD(TAG, "Loading the CA root certificate...");

    ret = mbedtls_x509_crt_parse(&n->cacert, (uint8_t*)server_root_cert, strlen(server_root_cert)+1);
    if(ret < 0)
    {
        ESP_LOGE(TAG, "mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        goto exit;
    }

    ESP_LOGD(TAG, "Setting hostname for TLS session...");

     /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&n->ssl, addr)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        goto exit;
    }

    ESP_LOGD(TAG, "Setting up the SSL/TLS structure...");

    if((ret = mbedtls_ssl_config_defaults(&n->conf,
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
    mbedtls_ssl_conf_authmode(&n->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&n->conf, &n->cacert, NULL);
    mbedtls_ssl_conf_rng(&n->conf, mbedtls_ctr_drbg_random, &n->ctr_drbg);

#ifdef MBEDTLS_DEBUG_C
    mbedtls_debug_set_threshold(MBEDTLS_DEBUG_LEVEL);
    mbedtls_ssl_conf_dbg(&n->conf, mbedtls_debug, NULL);
#endif
    if ((ret = mbedtls_ssl_setup(&n->ssl, &n->conf)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

	mbedtls_net_init(&n->server_fd);

	sprintf(portbuf,"%d",port);
    ESP_LOGD(TAG,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
	ESP_LOGI(TAG, "Connecting to %s:%s...", addr, portbuf);

	if ((ret = mbedtls_net_connect(&n->server_fd, addr,
								  portbuf, MBEDTLS_NET_PROTO_TCP)) != 0)
	{
		ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
		goto exit;
	}

	n->connect=1;
	ESP_LOGI(TAG, "Connected.");

	ret = mbedtls_net_set_block(&n->server_fd);

	mbedtls_ssl_set_bio(&n->ssl, &n->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    ESP_LOGD(TAG,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
	ESP_LOGD(TAG, "Performing the SSL/TLS handshake...");

	uint32_t hspos=0;
	while ((ret = mbedtls_ssl_handshake(&n->ssl)) != 0)	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
			goto exit;
		}
		ESP_LOGD(TAG, "SSL/TLS handshake...%u",hspos++);
	}

    ESP_LOGD(TAG,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
	ESP_LOGD(TAG, "Verifying peer X.509 certificate...");

	if ((flags = mbedtls_ssl_get_verify_result(&n->ssl)) != 0)
	{
		/* In real life, we probably want to close connection if ret != 0 */
		ESP_LOGW(TAG, "Failed to verify peer certificate!");
		bzero(n->ws_recvbuf, sizeof(n->ws_recvbuf));
		mbedtls_x509_crt_verify_info((char *)n->ws_recvbuf, sizeof(n->ws_recvbuf), "  ! ", flags);
		ESP_LOGW(TAG, "verification info: %s", n->ws_recvbuf);
	}
	else {
		ESP_LOGD(TAG, "Certificate verified.");
	}

    ESP_LOGD(TAG,"free DRAM %u IRAM %u",esp_get_free_heap_size(),xPortGetFreeHeapSizeTagged(MALLOC_CAP_32BIT));
	if (n->websocket) {
		unsigned char uuid[40];
		unsigned char uuid64[60];
		size_t uuidlen;

	    sprintf((char *)uuid,"%08x-%04x-%04x-%04x-%08x%04x",esp_random(),esp_random()&0xFFFF,esp_random()&0xFFFF,esp_random()&0xFFFF,esp_random(),esp_random()&0xFFFF);
		ESP_LOGD(TAG, "uuid=[%s]",uuid);
		bzero(uuid64, sizeof(uuid64));
		mbedtls_base64_encode(uuid64, sizeof(uuid64), &uuidlen,uuid, strlen((char *)uuid));
		ESP_LOGD(TAG, "uuid64=[%s]",uuid64);
		sprintf((char *)n->ws_sendbuf,"GET /mqtt HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nHost: %s:%d\r\nOrigin: https://%s:%d\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: mqtt\r\n\r\n",addr,port,addr,port,uuid64);

		ESP_LOGD(TAG, "req=[%s]",n->ws_sendbuf);

		while((ret = mbedtls_ssl_write(&n->ssl,n->ws_sendbuf,strlen((const char *)n->ws_sendbuf))) <= 0) {
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
				goto exit;
			}
		}

		len = ret;
		ESP_LOGD(TAG, "%d bytes written", len);
		ESP_LOGD(TAG, "Reading HTTP response...");

		do {
			bzero(n->ws_recvbuf, sizeof(n->ws_recvbuf));
			ret = mbedtls_ssl_read(&n->ssl, n->ws_recvbuf, MBEDTLS_WEBSOCKET_RECV_BUF_LEN-1);

			if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
				continue;
			}

			if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
				ESP_LOGD(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
				ret = 0;
				break;
			}

			if(ret < 0) {
				ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
				break;
			}

			if(ret == 0) {
				ESP_LOGD(TAG, "connection closed");
				break;
			}

			len = ret;
			ESP_LOGD(TAG, "%d bytes read", len);
#if defined(MBEDTLS_MQTT_DEBUG)
			/* Print response directly to stdout as it is read */
			for(int i = 0; i < len; i++) {
				putchar(n->ws_recvbuf[i]);
			}
#endif
			break;
		} while(1);

		if (strncmp((const char *)n->ws_recvbuf, "HTTP/1.1 101 Switching Protocols",32)!=0) {
			goto exit;
		}

		unsigned char buf[512];
		unsigned char hash[20];
		size_t buflen;

		memset(buf,0,sizeof(buf));
		sprintf((char *)buf,"%s%s",uuid64,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
		mbedtls_sha1(buf, strlen((const char*)buf), hash);
		mbedtls_base64_encode(buf, sizeof(buf), &buflen,hash, sizeof(hash));
#if defined(MBEDTLS_MQTT_DEBUG)
		printf("client hash %s\n",buf);
#endif

		if(strstr((char *)n->ws_recvbuf, (char *)buf)==NULL) {
			ESP_LOGE(TAG, "WebSocket handshake error, Sec-WebSocket-Accept invalid");
			goto exit;
		}



	}
	retVal = 0;

	return retVal;

exit:
	NetworkDisconnect(n);

	return retVal;
}

int NetworkSub(Network* n) {
    int ret = 0;
	int retVal = -1;
	int len;
    Timer timer;
    Timer ping_timer;

    TimerInit(&timer);
    TimerCountdownMS(&timer, 60000);

    //unsigned char bufsub[] = { 0x82,0x8C,0x01,0x02,0x03,0x04,0x83,0x08,0x03,0x05,0x01,0x07,0x73,0x7D,0x75,0x2D,0x20,0x04 };
	unsigned char bufsub[] = { 0x82,0x0A,0x00,0x01,0x00,0x05,0x70,0x79,0x74,0x2F,0x23,0x00 };

	if (!TimerIsExpired(&timer)) {
		ret = mqtt_mbedtls_write(n, bufsub, 12, TimerLeftMS(&timer));
	}
    TimerCountdown(&ping_timer, 60000); // record the fact that we have successfully sent the packet
	// ret = websocket_mbedtls_write(n, bufsub, 12);
	if(ret < 12) {
		ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
		goto exit;
	}
	len = ret;
	ESP_LOGI(TAG, "bufsub %d bytes written", len);
	ESP_LOGI(TAG, "Reading MQTT response...");

    buffered_mbedtls_read(n, 1000);
/*
	ESP_LOGI(TAG, "MQTT Yield response...");
	do {
		ret=buffered_mbedtls_read(n, 1000);
		if(ret <= 0) {
			ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
			goto exit;
		}
		for(int i = 4; i < ret; i++) {
			putchar(n->mqtt_recvbuf[i]);
		}
    	printf("\n");

	} while(1);
*/

	retVal = 0;

	return retVal;

exit:

	return retVal;
}

int NetworkYield(Network* n) {
    int ret = 0;
	int retVal = -1;

	do {
		ret=buffered_mbedtls_read(n,1000);
		if(ret <= 0) {
			ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
			goto exit;
		}
		for(int i = 4; i < ret; i++) {
			putchar(n->mqtt_recvbuf[i]);
		}
    	printf("\n");

	} while(1);

	retVal = 0;

	return retVal;

exit:
	return retVal;
}


void NetworkDisconnect(Network* n) {

	ESP_LOGI(TAG, "NetworkDisconnect");
	if (n->init==1) {
		ESP_LOGD(TAG, "mbedtls_ssl_close_notify");
		mbedtls_ssl_close_notify(&n->ssl);
		if (n->connect==1) {
			ESP_LOGD(TAG, "mbedtls_ssl_session_reset");
			mbedtls_ssl_session_reset(&n->ssl);
		}
		ESP_LOGD(TAG, "mbedtls_net_free");
		mbedtls_net_free(&n->server_fd);
		ESP_LOGD(TAG, "mbedtls_ssl_free");
		mbedtls_ssl_free(&n->ssl);
		ESP_LOGD(TAG, "mbedtls_ssl_config_free");
		mbedtls_ssl_config_free(&n->conf);
		ESP_LOGD(TAG, "mbedtls_ssl_config_free");
		mbedtls_entropy_free(&n->entropy);
		ESP_LOGD(TAG, "mbedtls_ctr_drbg_free");
		mbedtls_ctr_drbg_free(&n->ctr_drbg);
		ESP_LOGD(TAG, "mbedtls_x509_crt_free");
		mbedtls_x509_crt_free(&n->cacert);
	}
	n->connect=0;
	n->init=0;
    n->ws_recv_offset=0;
    n->ws_recv_len=0;
    n->mqtt_recv_offset=0;
    n->mqtt_recv_len=0;
}

