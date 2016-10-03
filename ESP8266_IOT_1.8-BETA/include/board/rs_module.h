/*
 ============================================================================
 Name        : rs_module.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_RS_MODULE_H_
#define SUPLA_RS_MODULE_H_


#define DS18B20
#define TEMPERATURE_CHANNEL 2

#ifdef __BOARD_rs_module
	#define LED_RED_PORT   14
#else
	#define LED_RED_PORT   12
#endif



#endif
