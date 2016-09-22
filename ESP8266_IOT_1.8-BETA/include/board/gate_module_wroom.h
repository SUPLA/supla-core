/*
 ============================================================================
 Name        : gate_module_wroom.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_GATE_MODULE_WROOM_H_
#define SUPLA_GATE_MODULE_WROOM_H_

#define DS18B20
#define TEMPERATURE_CHANNEL 4

#if defined(__BOARD_gate_module_wroom)

	#define LED_GREEN_PORT  5
	#define LED_BLUE_PORT   12

#else

	#define LED_GREEN_PORT  12
	#define LED_BLUE_PORT   14

#endif

#endif
