#!/bin/sh


case $1 in
   "thermometer_esp01")
   ;;
   "thermometer_esp01_ds_gpio0")
   ;;
   "wifisocket")
   ;;
   "wifisocket_esp01")
   ;;
   "wifisocket_54")
   ;;
   "gate_module")
   ;;
   "gate_module_esp01")
   ;;
   "gate_module_esp01_ds")
   ;;
   "gate_module_wroom")
   ;;
   "gate_module2_wroom")
   ;;
   "rs_module")
   ;;
   "rs_module_wroom")
   ;;
   "starter1_module_wroom")
   ;;
   "jangoe_rs")
   ;;
   *)
   echo "Usage:"
   echo "       build.sh BOARD_TYPE";
   echo "--------------------------";
   echo " Board types:             ";
   echo "              thermometer_esp01";
   echo "              thermometer_esp01_ds_gpio0";
   echo "              wifisocket  ";
   echo "              wifisocket_esp01";
   echo "              wifisocket_esp01_thermometer";
   echo "              wifisocket_54";
   echo "              gate_module";
   echo "              gate_module_esp01";
   echo "              gate_module_esp01_ds";
   echo "              gate_module_wroom";
   echo "              gate_module2_wroom";
   echo "              rs_module";
   echo "              rs_module_wroom";
   echo "              starter1_module_wroom";
   echo "              jangoe_rs";
   echo 
   echo
   exit;
   ;;
   
esac 

export PATH=/hdd2/Espressif/xtensa-lx106-elf/bin:$PATH
export COMPILE=gcc
export SDK_PATH=/hdd2/Espressif/ESP8266_IOT_SDK
export BIN_PATH=/hdd2/Espressif/ESP8266_BIN

make clean
make BOARD=$1 BOOT=new APP=0 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=0 && \
cp $BIN_PATH/eagle.flash.bin /media/sf_Public/$1_eagle.flash.bin &&
cp $BIN_PATH/eagle.irom0text.bin /media/sf_Public/$1_eagle.irom0text.bin &&
exit 0

exit 1
