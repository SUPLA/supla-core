/*
 ============================================================================
 Name        : sonoff.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SONOFF_H_
#define SONOFF_H_

#ifdef __BOARD_sonoff_ds18b20
	#define DS18B20
	#define TEMPERATURE_CHANNEL 1
	#define W1_GPIO3
#endif

#define LED_RED_PORT    13

#endif
