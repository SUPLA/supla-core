/*
 ============================================================================
 Name        : supla_esp_board.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#include "supla_esp.h"

#if defined(__BOARD_dht11_esp01) \
    || defined(__BOARD_dht22_esp01) \
    || defined(__BOARD_am2302_esp01) \

#include "board/dht_esp01.c"

#elif defined(__BOARD_thermometer_esp01) \
      || defined(__BOARD_thermometer_esp01_ds_gpio0) \

#include "board/thermometer_esp01.c"

#elif defined(__BOARD_wifisocket)  \
      || defined(__BOARD_wifisocket_x4) \
      || defined(__BOARD_wifisocket_54) \
      || defined(__BOARD_wifisocket_esp01)

#include "board/wifisocket.c"

#elif defined(__BOARD_gate_module_esp01)    \
      || defined(__BOARD_gate_module_esp01_ds)

#include "board/gate_module_esp01.c"

#elif defined(__BOARD_gate_module) \
      || defined(__BOARD_gate_module_dht11)    \
      || defined(__BOARD_gate_module_dht22)    \

#include "board/gate_module.c"

#elif defined(__BOARD_gate_module_wroom)    \
      || defined(__BOARD_gate_module2_wroom)

#include "board/gate_module_wroom.c"

#elif defined(__BOARD_rs_module) \
      || defined(__BOARD_rs_module_wroom)

#include "board/rs_module.c"

#elif defined(__BOARD_starter1_module_wroom)

#elif defined(__BOARD_jangoe_rs)

#elif defined(__BOARD_jangoe_wifisocket)

#elif defined(__BOARD_sonoff) \
      || defined(__BOARD_sonoff_ds18b20)

#include "board/sonoff.c"

#elif defined(__BOARD_dimmer)

#elif defined(__BOARD_EgyIOT)

#elif defined(__BOARD_zam_row_01)

#include "board/acs_zam_row_01.c"

#elif defined(__BOARD_lightswitch_x2) \
        || defined(__BOARD_lightswitch_x2_DHT11) \
        || defined(__BOARD_lightswitch_x2_DHT22) \
        || defined(__BOARD_lightswitch_x2_54) \
        || defined(__BOARD_lightswitch_x2_54_DHT11) \
        || defined(__BOARD_lightswitch_x2_54_DHT22)

#include "board/lightswitch.c"

#endif
