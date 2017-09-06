MQTT MySQL/MariaDB demo
===========================

mysql used
	mysql  Ver 15.1 Distrib 10.1.26-MariaDB, for debian-linux-gnu (x86_64)

Python used
	Python 2.7.13

Install (Change User and/or Password !)

	sudo apt-get install mysql-server mysql-client libmysqlclient-dev (libmariadbclient-dev)
	sudo apt-get install python-pip
	sudo pip install MySQL-python
	sudo pip install paho-mqtt

	mysql -u root -p
	(sysdo mysql -u root)

	SHOW DATABASES;

	CREATE DATABASE esp32_iot_mqtt_store;

	USE esp32_iot_mqtt_store

	CREATE TABLE mqtt_store
	(
	  id              INT unsigned NOT NULL AUTO_INCREMENT, # Unique ID for the record
	  topic           VARCHAR(1024) NOT NULL,               # MQTT Topic
	  msg             VARCHAR(1024) NOT NULL,               # MQTT Message
	  ts              TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  # Timestamp
	  PRIMARY KEY     (id)                                  # Make the id the primary key
	);

	SHOW TABLES;

	DESCRIBE mqtt_store;

	INSERT INTO mqtt_store ( topic, msg) VALUES
	  ( 'esp32/sensor/1/temp', '21.3' ),
	  ( 'esp32/sensor/2/hum', '87' );

	select * from mqtt_store;


	CREATE USER 'mqttsql'@'localhost' IDENTIFIED BY 'mqttpass';

	GRANT INSERT,SELECT ON esp32_iot_mqtt_store.mqtt_store TO 'mqttsql'@'localhost';

	exit

	mysql -u mqttsql -p

	use esp32_iot_mqtt_store;

	select * from mqtt_store;

	exit;

	
