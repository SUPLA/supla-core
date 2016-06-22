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
CFG_SECTOR=0x3C
NOSSL=0

case $1 in
   "dht11_esp01")
   ;;
   "dht22_esp01")
   ;;
   "am2302_esp01")
   ;;
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
   "gate_module_dht11")
   ;;
   "gate_module_dht22")
   ;;
   "gate_module_esp01")
   ;;
   "gate_module_esp01_ds")
   ;;
   "gate_module_wroom")
     CFG_SECTOR=0xBC
   ;;
   "gate_module2_wroom")
     CFG_SECTOR=0xBC
   ;;
   "rs_module")
   ;;
   "rs_module_wroom")
     CFG_SECTOR=0xBC
   ;;
   "starter1_module_wroom")
     CFG_SECTOR=0xBC
   ;;
   "jangoe_rs")
   ;;
   "sonoff")
     CFG_SECTOR=0xBC
   ;;
   "sonoff_ds18b20")
     CFG_SECTOR=0xBC
   ;;
   "EgyIOT")
     DEP_LIBS="-lpwm"
     NOSSL=1
   ;;
   "dimmer")
   ;;
   "zam_wop_01")
   ;;
   "rgbw")
     DEP_LIBS="-lpwm"
     NOSSL=1
   ;;
   "rgbw_wroom")
     DEP_LIBS="-lpwm -lssl"
     CFG_SECTOR=0xBC
     UPGRADE_1024=1
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
   echo "              rs_module_wroom";
   echo "              starter1_module_wroom";
   echo "              jangoe_rs";
   echo "              sonoff";
   echo "              sonoff_ds18b20";
   echo "              EgyIOT";
   echo "              dimmer";
   echo "              zam_wop_01";
   echo "              rgbw";
   echo "              rgbw_wroom";
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

BOARD_NAME=$1

if [ "$NOSSL" -eq 1 ]; then
  EXTRA="NOSSL=1"
  BOARD_NAME="$1"_nossl
else
  EXTRA="NOSSL=0"
fi

if [ "$UPGRADE_1024" -eq 1 ]; then

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
