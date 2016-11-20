/*
 ============================================================================
 Name        : gate_module.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_GATE_MODULE_H_
#define SUPLA_GATE_MODULE_H_


#if defined(__BOARD_gate_module_dht11) \
    || defined(__BOARD_gate_module_dht22)
   #define DHTSENSOR
   #define TEMPERATURE_HUMIDITY_CHANNEL 4
#else
   #define DS18B20
   #define TEMPERATURE_CHANNEL 4
#endif


#define LED_GREEN_PORT  12
#define LED_BLUE_PORT   14
#define LED_REINIT

#endif
