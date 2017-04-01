deps_config := \
	/home/pcbreflux/esp/esp-idf/components/aws_iot/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/bt/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/esp32/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/ethernet/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/fatfs/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/freertos/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/log/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/lwip/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/mbedtls/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/openssl/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/spi_flash/Kconfig \
	/home/pcbreflux/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/pcbreflux/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/pcbreflux/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/pcbreflux/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
