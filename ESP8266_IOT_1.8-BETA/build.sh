#!/bin/sh

###
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
# @author Przemyslaw Zygmunt przemek@supla.org
#
###

DEP_LIBS="-lssl"
CFG_SECTOR=0xBC
SDK154=1
UPGRADE_1024=1
NOSSL=0

export PATH=/hdd2/Espressif/xtensa-lx106-elf/bin:$PATH
export COMPILE=gcc
export SDK_PATH=/hdd2/Espressif/ESP8266_IOT_SDK
export BIN_PATH=/hdd2/Espressif/ESP8266_BIN


case $1 in
   "dht11_esp01")
      CFG_SECTOR=0x3C
      SDK154=0
      UPGRADE_1024=0
   ;;
   "dht22_esp01")
      CFG_SECTOR=0x3C
      SDK154=0
      UPGRADE_1024=0
   ;;
   "am2302_esp01")
      CFG_SECTOR=0x3C
      SDK154=0
      UPGRADE_1024=0
   ;;
   "thermometer_esp01")
      CFG_SECTOR=0x3C
      SDK154=0
      UPGRADE_1024=0
   ;;
   "thermometer_esp01_ds_gpio0")
      CFG_SECTOR=0x3C
      SDK154=0
      UPGRADE_1024=0
   ;;
   "wifisocket")
   ;;
   "wifisocket_x4")
   ;;
   "wifisocket_esp01")
      CFG_SECTOR=0x3C
      SDK154=0
      UPGRADE_1024=0
   ;;
   "wifisocket_54")
   ;;
   "gate_module")
   ;;
   "gate_module_dht11")
   ;;
   "gate_module_dht22")
   ;;
   "gate_module_esp01")
      CFG_SECTOR=0x3C
      SDK154=0
      UPGRADE_1024=0
   ;;
   "gate_module_esp01_ds")
      CFG_SECTOR=0x3C
      SDK154=0
      UPGRADE_1024=0
   ;;
   "gate_module_wroom")
   ;;
   "gate_module2_wroom")
   ;;
   "rs_module")
   ;;
   "starter1_module_wroom")
   ;;
   "jangoe_wifisocket")
   ;;
   "jangoe_rs")
   ;;
   "lightswitch_x2")
     UPGRADE_4096=1
   ;;
   "lightswitch_x2_54")
     UPGRADE_4096=1
   ;;
   "lightswitch_x2_DHT11")
     UPGRADE_4096=1
   ;;
   "lightswitch_x2_54_DHT11")
     UPGRADE_4096=1
   ;;
   "lightswitch_x2_DHT22")
     UPGRADE_4096=1
   ;;
   "lightswitch_x2_54_DHT22")
     UPGRADE_4096=1
   ;;
   "sonoff")
   ;;
   "sonoff_ds18b20")
   ;;
   "EgyIOT")
     DEP_LIBS="-lpwm"
     NOSSL=1
   ;;
   "dimmer")
     DEP_LIBS="-lpwm"
     NOSSL=1
   ;;
   "zam_row_01")
   ;;
   "rgbw_wroom")
     DEP_LIBS="-lpwm -lssl"
   ;;
   "h801")
     DEP_LIBS="-lpwm -lssl"
   ;;
   *)
   echo "Usage:"
   echo "       build.sh BOARD_TYPE";
   echo "--------------------------";
   echo " Board types:             ";
   echo "              dht11_esp01";
   echo "              dht22_esp01";
   echo "              am2302_esp01";
   echo "              thermometer_esp01";
   echo "              thermometer_esp01_ds_gpio0";
   echo "              wifisocket  ";
   echo "              wifisocket_x4";
   echo "              wifisocket_esp01";
   echo "              wifisocket_esp01_thermometer";
   echo "              wifisocket_54";
   echo "              gate_module";
   echo "              gate_module_dht11";
   echo "              gate_module_dht22";
   echo "              gate_module_esp01";
   echo "              gate_module_esp01_ds";
   echo "              gate_module_wroom";
   echo "              gate_module2_wroom";
   echo "              rs_module";
   echo "              starter1_module_wroom";
   echo "              jangoe_rs";
   echo "              jangoe_wifisocket";
   echo "              sonoff";
   echo "              sonoff_ds18b20";
   echo "              EgyIOT";
   echo "              dimmer";
   echo "              zam_row_01";
   echo "              rgbw_wroom";
   echo "              h801";
   echo "              lightswitch_x2";
   echo "              lightswitch_x2_54";
   echo "              lightswitch_x2_DHT11";
   echo "              lightswitch_x2_54_DHT11";
   echo "              lightswitch_x2_DHT22";
   echo "              lightswitch_x2_54_DHT22";
   echo 
   echo
   exit;
   ;;
   
esac 

if [ "$SDK154" -eq 1 ]; then
  export SDK_PATH=/hdd2/Espressif/ESP8266_NONOS_SDK154
  export BIN_PATH=/hdd2/Espressif/ESP8266_BIN154
fi

make clean

BOARD_NAME=$1

if [ "$NOSSL" -eq 1 ]; then
  EXTRA="NOSSL=1"
  BOARD_NAME="$1"_nossl
else
  EXTRA="NOSSL=0"
fi

if [ "$UPGRADE_4096" -eq 1 ]; then
   CFG_SECTOR=0xC0

   make SUPLA_DEP_LIBS="$DEP_LIBS"  BOARD=$1 CFG_SECTOR="$CFG_SECTOR" BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=4 $EXTRA 
   cp $BIN_PATH/upgrade/user1.4096.new.4.bin /media/sf_Public/"$BOARD_NAME"_user1.4096.new.4.bin && \
   cp $SDK_PATH/bin/boot_v1.2.bin /media/sf_Public/boot_v1.2.bin

elif [ "$UPGRADE_1024" -eq 1 ]; then

   make SUPLA_DEP_LIBS="$DEP_LIBS"  BOARD=$1 CFG_SECTOR="$CFG_SECTOR" BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=2 $EXTRA && \
   cp $BIN_PATH/upgrade/user1.1024.new.2.bin /media/sf_Public/"$BOARD_NAME"_user1.1024.new.2.bin && \
   cp $SDK_PATH/bin/boot_v1.2.bin /media/sf_Public/boot_v1.2.bin
   
else

   make SUPLA_DEP_LIBS="$DEP_LIBS" BOARD=$1 CFG_SECTOR=$CFG_SECTOR BOOT=new APP=0 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=0 $EXTRA && \
   cp $BIN_PATH/eagle.flash.bin /media/sf_Public/"$BOARD_NAME"_eagle.flash.bin && \
   cp $BIN_PATH/eagle.irom0text.bin /media/sf_Public/"$BOARD_NAME"_eagle.irom0text.bin &&
   
   exit 0
fi

exit 1
