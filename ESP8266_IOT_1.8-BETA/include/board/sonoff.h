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

    // Sonoff TH (v1)
	//#define W1_GPIO3

    // Sonoff TH (v2)
    #define W1_GPIO14
#endif

#define LED_RED_PORT    13

#endif
