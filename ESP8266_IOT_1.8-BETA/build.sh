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
SDK154=1
NOSSL=0

export PATH=/hdd2/Espressif/xtensa-lx106-elf/bin:$PATH
export COMPILE=gcc
export SDK_PATH=/hdd2/Espressif/ESP8266_IOT_SDK
export BIN_PATH=/hdd2/Espressif/ESP8266_BIN

case $1 in
   "dht11_esp01")
      FLASH_SIZE="512"
   ;;
   "dht22_esp01")
      FLASH_SIZE="512"
   ;;
   "am2302_esp01")
      FLASH_SIZE="512"
   ;;
   "thermometer_esp01")
      FLASH_SIZE="512"
   ;;
   "thermometer_esp01_ds_gpio0")
      FLASH_SIZE="512"
   ;;
   "wifisocket")
   ;;
   "wifisocket_x4")
   ;;
   "wifisocket_esp01")
      FLASH_SIZE="512"
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
      FLASH_SIZE="512"
   ;;
   "gate_module_esp01_ds")
      FLASH_SIZE="512"
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
     FLASH_SIZE="4096"
   ;;
   "lightswitch_x2_54")
     FLASH_SIZE="4096"
   ;;
   "lightswitch_x2_DHT11")
     FLASH_SIZE="4096"
   ;;
   "lightswitch_x2_54_DHT11")
     FLASH_SIZE="4096"
   ;;
   "lightswitch_x2_DHT22")
     FLASH_SIZE="4096"
   ;;
   "lightswitch_x2_54_DHT22")
     FLASH_SIZE="4096"
   ;;
   "sonoff")
   ;;
   "sonoff_socket")
   ;;
   "sonoff_touch")
      SPI_MODE="DOUT"
   ;;
   "sonoff_th16")
   ;;
   "sonoff_th10")
   ;;
   "sonoff_dual")
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
   "zam_sbp_01")
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
   echo "              sonoff_touch";
   echo "              sonoff_socket";
   echo "              sonoff_th10";
   echo "              sonoff_th16";
   echo "              sonoff_dual";
   echo "              EgyIOT";
   echo "              dimmer";
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

case $FLASH_SIZE in
   "512")
     CFG_SECTOR=0x3C
     SDK154=0
   ;;
   "4096")
     CFG_SECTOR=0xC0
   ;;
   *)
     FLASH_SIZE="1024"
     CFG_SECTOR=0xBC
   ;;
esac


if [ -z "$SPI_MODE" ]; then
  SPI_MODE = "QIO"
fi

if [ "$SDK154" -eq 1 ]; then
  export SDK_PATH=/hdd2/Espressif/ESP8266_NONOS_SDK154
  export BIN_PATH=/hdd2/Espressif/ESP8266_BIN154

  cp ./ld/sdk154/"$FLASH_SIZE"_eagle.app.v6.ld $SDK_PATH/ld/eagle.app.v6.ld || exit 1
fi

make clean

BOARD_NAME=$1

if [ "$NOSSL" -eq 1 ]; then
  EXTRA="NOSSL=1"
  BOARD_NAME="$1"_nossl
else
  EXTRA="NOSSL=0"
fi



   make SUPLA_DEP_LIBS="$DEP_LIBS" BOARD=$1 CFG_SECTOR=$CFG_SECTOR BOOT=new APP=0 SPI_SPEED=40 SPI_MODE="$SPI_MODE" SPI_SIZE_MAP=0 $EXTRA && \
   cp $BIN_PATH/eagle.flash.bin /media/sf_Public/"$BOARD_NAME"_"$FLASH_SIZE"_eagle.flash.bin && \
   cp $BIN_PATH/eagle.irom0text.bin /media/sf_Public/"$BOARD_NAME"_"$FLASH_SIZE"_eagle.irom0text.bin &&
   
   exit 0

exit 1
