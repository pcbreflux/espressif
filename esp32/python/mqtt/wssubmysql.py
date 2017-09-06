# Copyright (c) 2017 pcbreflux. All Rights Reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>. *
#
from __future__ import print_function
import paho.mqtt.subscribe as subscribe
#import psutil # included in newer python versions
import sys
import ssl
import MySQLdb
###   Start of user configuration   ###   
#  MQTT Connection Methods
mqttHost = "xxx.cloudmqtt.com"
#tTransport = "websockets"
#tPort = 345678
tTLS = tTLS = {'ca_certs':"/etc/ssl/certs/ca-certificates.crt",'tls_version':ssl.PROTOCOL_TLSv1_2}
tTransport = "tcp"
tPort = 23456
#tTLS = None
topics = ["esp32/#"]
db = MySQLdb.connect(host="localhost",            # your host, usually localhost
                     user="mqttsql",              # your username
                     passwd="mqttpass",           # your password
                     db="esp32_iot_mqtt_store")   # name of the data base
cursor = db.cursor()
add_mqtt = ("INSERT INTO mqtt_store (topic, msg) VALUES (%s, %s)")
tpos = 0
while(True):
    try:
        m = subscribe.simple(topics, hostname=mqttHost, port=tPort, client_id="ESP32MQTTWS", retained=True, msg_count=1, auth={'username':"sensor", 'password':"password"}, tls=tTLS, transport=tTransport)
        data_mqtt = (m.topic,m.payload)
        cursor.execute(add_mqtt, data_mqtt)
        msg_no = cursor.lastrowid
        db.commit()
        print(str(msg_no)+"/"+str(tpos) +" "+m.topic+"|"+m.payload)
        tpos += 1
    except (KeyboardInterrupt):
        cursor.close()
        db.close()
        break
    except:
        cursor.close()
        db.close()
        print (sys.exc_info())
        print ("There was an error while subscribe the data.")
