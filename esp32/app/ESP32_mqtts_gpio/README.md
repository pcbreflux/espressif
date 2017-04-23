# MQTT Secure via HTTPS Websocket Example

Uses an mbedTLS socket to make a very simple HTTPS request over a secure connection, including verifying the server TLS certificate.
You have to use a seperate MQTT Broker. Setting up your own like mosquitto (with SSL/TLS) maybe on raspberian or ubuntu or use one on the Cloud like iot.eclipse.org (even encpryted, this is public!), 
test.mosquitto.org (also public) or coudmqtt.com (your private one) or others. 

## Example MQTT
* Send Topic esp32/bulb1/0 Message 50 to switch first GPIO (-> 5) in the List (5,18,23,19,22) to 50& duty cycle -> 50% on / 50% of frequency 1kHz
* Send Topic esp32/bulb1/1 Message 0 to switch first GPIO 18 OFF
* Send Topic esp32/bulb1/3 Message 100 to switch first GPIO 19 ON (-> 100%)

## esp-idf used
* commit fd3ef4cdfe1ce747ef4757205f4bb797b099c9d9
* Merge: 94a61389 52c378b4
* Author: Angus Gratton <angus@espressif.com>
* Date:   Fri Apr 21 12:27:32 2017 +0800


## eclipse
* include include.xml to C-Paths

## PREPARE
1. change main/cert.c -> server_root_cert
2. change main/mqtt_subscribe_main.c
    * `#define MQTT_SERVER "your server"`
    * `#define MQTT_USER "your user"`
    * `#define MQTT_PASS "your password"`
    * `#define MQTT_PORT your port`
    * `//#define MQTT_WEBSOCKET 0  // 0=no 1=yes`
    * `#define MQTT_WEBSOCKET 1  // 0=no 1=yes`

(Hint: today mbed_ssl without WebSockets is unstable, i.e. reconnecting is needed some times, searching for reasons...)

## INSTALL
* `make menuconfig`
* `make -j8 all`
* `make flash`
* `make monitor`




