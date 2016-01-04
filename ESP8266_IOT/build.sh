#!/bin/sh

export PATH=/hdd2/Espressif/xtensa-lx106-elf/bin:$PATH
export COMPILE=gcc
export SDK_PATH=/hdd2/Espressif/ESP8266_IOT_SDK
export BIN_PATH=/hdd2/Espressif/ESP8266_BIN


make clean
make BOOT=new APP=0 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=0 && \
cp $BIN_PATH/eagle.flash.bin /media/sf_Public/ &&
cp $BIN_PATH/eagle.irom0text.bin /media/sf_Public/ &&
exit 0

exit 1
