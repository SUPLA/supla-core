/*
 ============================================================================
 Name        : thermometer_esp01.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef THERMOMETER_ESP01_H_
#define THERMOMETER_ESP01_H_

#define DS18B20
#define TEMPERATURE_CHANNEL 0

#ifdef __BOARD_thermometer_esp01_ds_gpio0
	#define W1_GPIO0
#endif

#endif
