/*
 ============================================================================
 Name        : wifisocket.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef WIFISOCKET_H_
#define WIFISOCKET_H_

#ifdef __BOARD_wifisocket_x4

	#define USE_GPIO16_OUTPUT

#else
	#ifndef __BOARD_wifisocket_esp01

		#define DS18B20
		#define CFGBTN_TYPE_SELECTION
		#define TEMPERATURE_CHANNEL 1
		#define LED_RED_PORT    13
		#define LED_GREEN_PORT  12
		#define LED_BLUE_PORT   14

	#endif
#endif

#endif
