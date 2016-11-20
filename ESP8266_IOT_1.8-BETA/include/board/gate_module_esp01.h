/*
 ============================================================================
 Name        : gate_module_esp01.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_GATE_MODULE_ESP01_H_
#define SUPLA_GATE_MODULE_ESP01_H_


#ifdef __BOARD_gate_module_esp01_ds
   #define DHTSENSOR
   #define TEMPERATURE_HUMIDITY_CHANNEL 1
   #define USE_GPIO3
#endif


#endif
