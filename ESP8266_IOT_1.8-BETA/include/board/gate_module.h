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
#else
   #define DS18B20
#endif

#define TEMPERATURE_CHANNEL 4

#define LED_GREEN_PORT  12
#define LED_BLUE_PORT   14

#endif
