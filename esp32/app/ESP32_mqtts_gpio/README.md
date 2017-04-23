# MQTT Secure via HTTPS Websocket Example

Uses an mbedTLS socket to make a very simple HTTPS request over a secure connection, including verifying the server TLS certificate.

esp-idf used
	commit fd3ef4cdfe1ce747ef4757205f4bb797b099c9d9
	Merge: 94a61389 52c378b4
	Author: Angus Gratton <angus@espressif.com>
	Date:   Fri Apr 21 12:27:32 2017 +0800


eclipse
	inplude include.xml to C-Paths

PREPARE
	change main/cert.c -> server_root_cert
	change main/mqtt_subscribe_main.c
		#define MQTT_SERVER "your server"
		#define MQTT_USER "your user"
		#define MQTT_PASS "your password"
		#define MQTT_PORT your port
		//#define MQTT_WEBSOCKET 0  // 0=no 1=yes
		#define MQTT_WEBSOCKET 1  // 0=no 1=yes

(Hint: today mbed_ssl without WebSockets unstable, searching for reasons...)

INSTALL
	make menuconfig
	make -j8 all
	make flash
	make monitor

