# GATTS MQTT Gateway Example

uses
	xtensa 5.2.0
	commit 4745895e229d8e1ca4e6f4b601397c0ba0b3d2db Wed Mar 8 17:37:09 2017 +0800

Install:
   Change your Settings in mqtt_task.c:
	#define MQTT_SERVER "abc.cloudmqtt.com"
	#define MQTT_USER "user"
	#define MQTT_PASS "pass"
	#define MQTT_PORT 12345
	#define MQTT_WEBSOCKET 1  // 0=no 1=yes
   Create the servercertifikate in cert.c
	openssl s_client -showcerts -connect abc.cloudmqtt.com:12345 </dev/null

   make menuconfig
	config your CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD
   make -j8 all
   make flash monitor


