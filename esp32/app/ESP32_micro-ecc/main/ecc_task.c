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
#include "ecc_task.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "uECC.h"
#include "hwcrypto/aes.h"
#include "hwcrypto/sha.h"
#include "ecc_task.h"

#include "sdkconfig.h"


#define TAG "uECC"

#define SECRET_SIZE 32
#define MESSAGE_SIZE 1024
#define MESSAGE_HASH_SIZE 48
#define SIGN_SIZE 64
#define KEY_ROUND_SIZE 256

static int rng_func(uint8_t *dest, unsigned size) {

    uint32_t rand=0;
    unsigned pos=0;
    uint8_t step=0;
    uint8_t rand8=0;

    while (pos<size) {
    	if (step>=4) {
    		step=0;
    	}
    	if (step==0) {
    		rand=esp_random();
        	// ESP_LOGI(TAG, "rand 0x%08X",rand);
    	}
    	// faster then 8*step ?
        switch(step) {
    		case 0:
    		   	rand8=rand&0xFF;
    		   	break;
    		case 1:
    		   	rand8=(rand>>8)&0xFF;
    		   	break;
    		case 2:
    		   	rand8=(rand>>16)&0xFF;
    		   	break;
    		case 3:
    		   	rand8=(rand>>24)&0xFF;
    		   	break;
        }
    	// ESP_LOGI(TAG, "%d) rand 8 0x%02X",pos,rand8);
		*dest++=rand8;
    	step++;
    	pos++;
    }

    return 1; // random data was generated
}


void ecc_task(void *pvParameters) {
	uint32_t loop=0;
	uECC_Curve p_curve;  // eliptic curve
	uint8_t private_key1[32];  // local secret key
	uint8_t public_key1[64];   // local public key
	uint8_t public_key_comp[64]; // computed key
	uint8_t private_key2[32];  // remote secret key
	uint8_t public_key2[64];   // remote public key
	uint8_t secret1[SECRET_SIZE];  // local shared secret
	uint8_t secret2[SECRET_SIZE];  // remote shared secret
	uint8_t message[MESSAGE_SIZE]; // public message
	uint8_t encrypt_message[MESSAGE_SIZE]; // public message
	uint8_t decrypt_message[MESSAGE_SIZE]; // public message
	uint8_t message_hash[MESSAGE_HASH_SIZE]; // sha256 hash of message
	uint8_t signature[SIGN_SIZE]; // signature of message hash
	uint32_t key_round[KEY_ROUND_SIZE]; // AES round keys
    unsigned char nonce[8];
    unsigned char nonce_counter[16];
    unsigned char stream_block[16];
	int ret;
	esp_aes_context ctx;
	//esp_aes_context *p_ctx=&ctx;
	size_t nc_off;

    p_curve = uECC_secp256r1();

    uECC_set_rng(rng_func);

    int privksize;

    int pubksize;

   	esp_aes_init(&ctx);
   	ctx.nr=KEY_ROUND_SIZE;
   	ctx.rk=key_round;


	while (1) {
	   	rng_func(message,MESSAGE_SIZE);  // fill message with random
	   	rng_func(nonce,8);  // fill nonce with random, the nonce have to be shared between local and remote
	   	ret = uECC_make_key(public_key1, private_key1,p_curve);
	   	ret = uECC_make_key(public_key2, private_key2,p_curve);

	   	ESP_LOGI(TAG, "---------------------------- %d",loop);
		privksize = uECC_curve_private_key_size(p_curve);
		pubksize = uECC_curve_public_key_size(p_curve);
//	   	ESP_LOGI(TAG, "priv=%d pub=%d",privksize,pubksize);
//	   	ESP_LOGI(TAG, "---private1---");
//	   	printf("private_key = {\n");
//	   	for (int i=0;i<privksize;i++) {
//	   		printf("0x%02X,",private_key1[i]);
//	   	}
//	   	printf("\n}\n");
//	   	ESP_LOGI(TAG, "---public1---");
//	   	printf("public_key = {\n");
//	   	for (int i=0;i<pubksize;i++) {
//	   		printf("0x%02X,",public_key2[i]);
//	   	}
//	   	printf("\n};\n");

	   	uECC_compute_public_key(private_key1, public_key_comp, p_curve);

//	   	ESP_LOGI(TAG, "---public1---");
//	   	for (int i=0;i<pubksize;i++) {
//	   		printf("0x%02X,",public_key_comp[i]);
//	   	}
//	   	printf("\n");

//	   	ESP_LOGI(TAG, "---private2---");
//	   	printf("private_key = {\n");
//	   	for (int i=0;i<privksize;i++) {
//	   		printf("0x%02X,",private_key2[i]);
//	   	}
//	   	printf("\n}\n");
//	   	ESP_LOGI(TAG, "---public2---");
//	   	printf("public_key = {\n");
//	   	for (int i=0;i<pubksize;i++) {
//	   		printf("0x%02X,",public_key2[i]);
//	   	}
//	   	printf("\n};\n");

	   	uECC_compute_public_key(private_key2, public_key_comp, p_curve);

//	   	ESP_LOGI(TAG, "---public2---");
//	   	for (int i=0;i<pubksize;i++) {
//	   		printf("0x%02X,",public_key_comp[i]);
//	   	}
//	   	printf("\n");

	   	ret=uECC_shared_secret(public_key2,private_key1,secret1,p_curve);

//	   	ESP_LOGI(TAG, "---secret1---");
//	   	printf("secret1 = {\n");
//	   	for (int i=0;i<SECRET_SIZE;i++) {
//	   		printf("0x%02X,",secret1[i]);
//	   	}
//	   	printf("\n}\n");

	   	ret=uECC_shared_secret(public_key1,private_key2,secret2,p_curve);

//	   	ESP_LOGI(TAG, "---secret2---");
//	   	printf("secret2 = {\n");
//	   	for (int i=0;i<SECRET_SIZE;i++) {
//	   		printf("0x%02X,",secret2[i]);
//	   	}
//	   	printf("\n}\n");

		ret=memcmp(secret1,secret2,SECRET_SIZE);

	   	if (ret==0) {
		   	ESP_LOGI(TAG, "---secret valid---");
	   	} else {
		   	ESP_LOGE(TAG, "---secret void---");
	   	}

	   	//	   	ESP_LOGI(TAG, "---message---");
//	   	printf("message = {\n");
//	   	for (int i=0;i<MESSAGE_SIZE;i++) {
//	   		printf("0x%02X,",message[i]);
//	   	}
//	   	printf("\n}\n");

	   	// use hardware for computing hash
	   	esp_sha(SHA2_256, (const unsigned char *)message, MESSAGE_SIZE, (unsigned char *)message_hash);

//	   	ESP_LOGI(TAG, "---message hash---");
//	   	printf("message_hash = {\n");
//	   	for (int i=0;i<MESSAGE_HASH_SIZE;i++) {
//	   		printf("0x%02X,",message_hash[i]);
//	   	}
//	   	printf("\n}\n");

	   	ret = uECC_sign(private_key1,message_hash,MESSAGE_HASH_SIZE,signature,p_curve);

//	   	ESP_LOGI(TAG, "---signature---");
//	   	printf("signature = {\n");
//	   	for (int i=0;i<SIGN_SIZE;i++) {
//	   		printf("0x%02X,",signature[i]);
//	   	}
//	   	printf("\n}\n");

	   	ret = uECC_verify(public_key1,message_hash,MESSAGE_HASH_SIZE,signature,p_curve);

	   	if (ret==1) {
		   	ESP_LOGI(TAG, "---signature valid---");
	   	} else {
		   	ESP_LOGE(TAG, "---signature void---");
	   	}

//		ESP_LOGI(TAG, "---message---");
//		printf("message = {\n");
//		for (int i=0;i<MESSAGE_SIZE;i++) {
//			printf("0x%02X,",message[i]);
//		}
//		printf("\n}\n");

	   	ctx.enc.aesbits=AES256;
	   	memcpy(ctx.enc.key,secret1,SECRET_SIZE);

	   	nc_off=0; // offset in the current stream_block
	   	bzero(nonce_counter,16);
	   	memcpy(nonce_counter,nonce,8);  // fill nonce with prepared random, couter stay 0
		esp_aes_crypt_ctr(&ctx,MESSAGE_SIZE,&nc_off,nonce_counter,stream_block,message,encrypt_message);

//	   	ESP_LOGI(TAG, "---encrypt_message---");
//		printf("encrypt_message = {\n");
//		for (int i=0;i<MESSAGE_SIZE;i++) {
//			printf("0x%02X,",encrypt_message[i]);
//		}
//		printf("\n}\n");

	   	ctx.enc.aesbits=AES256;
	   	memcpy(ctx.enc.key,secret2,SECRET_SIZE);

	   	nc_off=0; // offset in the current stream_block
	   	bzero(nonce_counter,16);
	   	memcpy(nonce_counter,nonce,8);  // fill nonce with prepared random, couter stay 0
		esp_aes_crypt_ctr(&ctx,MESSAGE_SIZE,&nc_off,nonce_counter,stream_block,encrypt_message,decrypt_message);

//	   	ESP_LOGI(TAG, "---decrypt_message---");
//		printf("decrypt_message = {\n");
//		for (int i=0;i<MESSAGE_SIZE;i++) {
//			printf("0x%02X,",decrypt_message[i]);
//		}
//		printf("\n}\n");

		ret=memcmp(message,decrypt_message,MESSAGE_SIZE);

	   	if (ret==0) {
		   	ESP_LOGI(TAG, "---decrypt valid---");
	   	} else {
		   	ESP_LOGE(TAG, "---decrypt void---");
	   	}

	   	vTaskDelay(1 / portTICK_RATE_MS);
		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
