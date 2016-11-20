/*
 ============================================================================
 Name        : lightswitch.h
 Author      : Zybi
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef LIGHTSWITCH_H_
#define LIGHTSWITCH_H_

#define B_RELAY2_PORT	13
#define B_BTN2_PORT	12

#define CFGBTN_TYPE_SELECTION
#define LED_RED_PORT    14

#if defined(__BOARD_lightswitch_x2_54) \
	|| defined(__BOARD_lightswitch_x2_54_DHT11) \
	|| defined(__BOARD_lightswitch_x2_54_DHT22)

	#define B_RELAY1_PORT	5
	#define B_CFG_PORT	4
#else
	#define B_RELAY1_PORT	4
	#define B_CFG_PORT	5
#endif


#if defined(__BOARD_lightswitch_x2) || defined(__BOARD_lightswitch_x2_54)

	#define DS18B20
        #define TEMPERATURE_CHANNEL 2

 #elif defined(__BOARD_lightswitch_x2_DHT11) || defined(__BOARD_lightswitch_x2_54_DHT11)

	#define DHTSENSOR
	#define SENSOR_DHT11
        #define TEMPERATURE_HUMIDITY_CHANNEL 2

 #elif defined(__BOARD_lightswitch_x2_DHT22) || defined(__BOARD_lightswitch_x2_54_DHT22)

	#define DHTSENSOR
	#define SENSOR_DHT22
        #define TEMPERATURE_HUMIDITY_CHANNEL 2

#endif


#endif // LIGHTSWITCH_H_

