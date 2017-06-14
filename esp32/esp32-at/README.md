ESP32 AT
===========================
commit b942d812bcdc8ba5ca502997cb7a69cb619704c9
Date:   Tue Feb 21 17:46:10 2017 +0800

esp-idf used
	commit 1e0710f1b24429a316c9c34732aa17bd3f189421
	Date:   Fri May 12 18:23:20 2017 +0800

Flash

	esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0xf000 phy_init_data.bin 0x10000 esp-at.bin 0x8000 partitions_at.bin

