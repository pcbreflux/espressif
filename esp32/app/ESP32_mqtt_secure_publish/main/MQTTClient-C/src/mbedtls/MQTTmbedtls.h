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
 *    pcbreflux - mbedtls implementation via tls and websocket
 *******************************************************************************/

#if !defined(MQTTmbedtls_H)
#define MQTTmbedtls_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

//#include "mbedtls/config.h"
#include "mbedtls/platform.h"
#include "mbedtls/debug.h"
#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/sha1.h"
#include "mbedtls/base64.h"


#include "lwip/sockets.h"

#include "esp_system.h"
#include "esp_log.h"

#include "mqtt_uuid.h"

#define WEBSOCKET_CONTINUATION 0x00
#define WEBSOCKET_TEXT 0x01
#define WEBSOCKET_BINARY 0x02
#define WEBSOCKET_CONNCLOSE 0x08
#define WEBSOCKET_PING 0x09
#define WEBSOCKET_PONG 0x0A

#define MBEDTLS_WEBSOCKET_SEND_BUF_LEN       1024
#define MBEDTLS_WEBSOCKET_RECV_BUF_LEN       1024
#define MBEDTLS_MQTT_RECV_BUF_LEN            1024
//#define MBEDTLS_MQTT_DEBUG

typedef struct Timer
{
	struct timeval end_time;
} Timer;

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

typedef struct Network
{
	mbedtls_ssl_context ssl;
    mbedtls_net_context server_fd;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;

    unsigned char ws_sendbuf[MBEDTLS_WEBSOCKET_SEND_BUF_LEN];
    unsigned char ws_recvbuf[MBEDTLS_WEBSOCKET_RECV_BUF_LEN];
    unsigned char mqtt_recvbuf[MBEDTLS_MQTT_RECV_BUF_LEN];
    int ws_recv_offset;
    int ws_recv_len;
    int mqtt_recv_offset;
    int mqtt_recv_len;
    uint8_t websocket;

	int (*mqttread) (struct Network*, unsigned char*, int, int);
	int (*mqttwrite) (struct Network*, unsigned char*, int, int);
} Network;

int mqtt_mbedtls_read(Network*, unsigned char*, int, int);
int mqtt_mbedtls_write(Network*, unsigned char*, int, int);

void NetworkInit(Network*);
int NetworkConnect(Network*, char*, int);
void NetworkDisconnect(Network*);
int NetworkSub(Network*);
int NetworkYield(Network*);

#endif

