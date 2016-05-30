/*
 ============================================================================
 Name        : supla_esp.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.3
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_H_
#define SUPLA_ESP_H_

#include "supla-dev/proto.h"

#define STD_CFG_SECTOR    0x3C
#define WROOM_CFG_SECTOR  0xBC

#define RELAY_LO_VALUE  0
#define RELAY_HI_VALUE  1


#define GPIO_PORT_INIT \
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); \
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5); \
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); \
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); \
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14)

#if defined(__BOARD_dht11_esp01)

	#define DEVICE_NAME "SUPLA-DHT11-ESP01"
	#define SENSOR_DHT11
    #define TEMPERATURE_CHANNEL 0
	#define BTN_PORT         0

#elif defined(__BOARD_dht22_esp01)

	#define DEVICE_NAME "SUPLA-DHT22-ESP01"
	#define SENSOR_DHT22
    #define TEMPERATURE_CHANNEL 0
	#define BTN_PORT         0

#elif defined(__BOARD_am2302_esp01)

	#define DEVICE_NAME "SUPLA-AM2302-ESP01"
	#define SENSOR_DHT22
    #define TEMPERATURE_CHANNEL 0
	#define BTN_PORT         0


#elif defined(__BOARD_thermometer_esp01)

	#define DEVICE_NAME "SUPLA-THERMOMETER-ESP01"
	#define DS18B20
    #define TEMPERATURE_CHANNEL 0
	#define BTN_PORT         0

#elif defined(__BOARD_thermometer_esp01_ds_gpio0)

	#define DEVICE_NAME "SUPLA-THERMOMETER-ESP01"
	#define DS18B20
	#define W1_GPIO0
	#define TEMPERATURE_CHANNEL 0
	#define BTN_PORT         2

#elif defined(__BOARD_wifisocket) || defined(__BOARD_wifisocket_54)

	#define DS18B20
    #define BTN_TYPE_SELECTION
	#define DEVICE_NAME "SUPLA-SOCKET"
	#define TEMPERATURE_CHANNEL 1
	#define LED_RED_PORT    13
	#define LED_GREEN_PORT  12
	#define LED_BLUE_PORT   14

	#ifdef __BOARD_wifisocket_54
		#define RELAY1_PORT      5
		#define BTN_PORT         4
	#else
		#define RELAY1_PORT      4
		#define BTN_PORT         5
	#endif

#elif defined(__BOARD_wifisocket_esp01)

	#define DEVICE_NAME "SUPLA-SOCKET"
	#define RELAY1_PORT      0
	#define BTN_PORT         2

#elif defined(__BOARD_gate_module_esp01)

	#define DEVICE_NAME "SUPLA-GATE-MODULE-ESP01"
	#define RELAY1_PORT      0
	#define BTN_PORT         2

#elif defined(__BOARD_gate_module_esp01_ds)

	#define DEVICE_NAME "SUPLA-GATE-MODULE-ESP01"
	#define RELAY1_PORT      3
	#define BTN_PORT         0
    #define TEMPERATURE_CHANNEL 1
	#define USE_GPIO3
	#define DS18B20

#elif defined(__BOARD_gate_module) || defined(__BOARD_gate_module_dht11) || defined(__BOARD_gate_module_dht22)

	#define DEVICE_NAME "SUPLA-GATE-MODULE"

    #if defined(__BOARD_gate_module_dht11)
       #define SENSOR_DHT11
    #elif defined(__BOARD_gate_module_dht22)
       #define SENSOR_DHT22
    #else
       #define DS18B20
    #endif

    #define RESET_RELAY_PORT
	#define TEMPERATURE_CHANNEL 4

	#define LED_GREEN_PORT  12
	#define LED_BLUE_PORT   14

	#define BTN_PORT         5

	#define RELAY1_PORT      4
	#define RELAY2_PORT      13

	#define INPUT_PORT1      12
	#define INPUT_PORT2      14

#elif defined(__BOARD_gate_module_wroom)

    #define CFG_SECTOR          WROOM_CFG_SECTOR
	#define DEVICE_NAME "SUPLA-GATE-MODULE"
    #define RESET_RELAY_PORT
    #define DS18B20
	#define TEMPERATURE_CHANNEL 4

	#define LED_GREEN_PORT  5
	#define LED_BLUE_PORT   12

	#define BTN_PORT         13

	#define RELAY1_PORT      4
	#define RELAY2_PORT      14

	#define INPUT_PORT1      5
	#define INPUT_PORT2      12

#elif defined(__BOARD_gate_module2_wroom)

    #define CFG_SECTOR    WROOM_CFG_SECTOR
	#define DEVICE_NAME "SUPLA-GATE-MODULE"
    #define RESET_RELAY_PORT
    #define DS18B20
	#define TEMPERATURE_CHANNEL 4

	#define LED_GREEN_PORT  12
	#define LED_BLUE_PORT   14

	#define BTN_PORT         13
	#define RELAY1_PORT      4
	#define RELAY2_PORT      5

	#define INPUT_PORT1      12
	#define INPUT_PORT2      14

#elif defined(__BOARD_rs_module)

	#define DEVICE_NAME "SUPLA-RS-MODULE"
    #define RESET_RELAY_PORT
    #define DS18B20
	#define TEMPERATURE_CHANNEL 2

	#define LED_GREEN_PORT  12
	#define LED_BLUE_PORT   14

	#define BTN_PORT         5

	#define RELAY1_PORT      4
	#define RELAY2_PORT      13

	#define INPUT_PORT1      12

#elif defined(__BOARD_rs_module_wroom)

    #define CFG_SECTOR   WROOM_CFG_SECTOR
	#define DEVICE_NAME "SUPLA-RS-MODULE"
    #define RESET_RELAY_PORT
    #define DS18B20

	#define TEMPERATURE_CHANNEL 2

	#define LED_GREEN_PORT  5
	#define LED_BLUE_PORT   12

	#define BTN_PORT         13
	#define RELAY1_PORT      4
	#define RELAY2_PORT      14

	#define INPUT_PORT1      5

#elif defined(__BOARD_starter1_module_wroom)

    #define CFG_SECTOR          WROOM_CFG_SECTOR
    #define DS18B20
    #define BTN_PULLUP
    #define W1_GPIO5
	#define DEVICE_NAME "SUPLA-STARTER1-MODULE"

    #define TEMPERATURE_CHANNEL 2

    #define LED_RED_PORT  14

	#define BTN_PORT         12
	#define RELAY1_PORT      4
	#define RELAY2_PORT      13

#elif defined(__BOARD_jangoe_rs)

	#define DEVICE_NAME "JANGOE-RS-MODULE"
    #define RESET_RELAY_PORT

    #define INPUT_PORT1      4

	#define BTN_PORT         5

	#define RELAY1_PORT      14
	#define RELAY2_PORT      2


    #undef GPIO_PORT_INIT

	#define GPIO_PORT_INIT \
    	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14)

    #undef RELAY_LO_VALUE
    #undef RELAY_HI_VALUE

	#define RELAY_LO_VALUE  1
	#define RELAY_HI_VALUE  0

#elif defined(__BOARD_sonoff)

    #define CFG_SECTOR          WROOM_CFG_SECTOR
	#define DEVICE_NAME "SONOFF"
	#define RELAY1_PORT      12
	#define BTN_PORT         0
    #define LED_RED_PORT     13

#elif defined(__BOARD_sonoff_ds18b20)

    #define CFG_SECTOR          WROOM_CFG_SECTOR
    #define DS18B20
	#define DEVICE_NAME "SONOFF-DS18B20"
	#define RELAY1_PORT      12
	#define BTN_PORT         0
    #define LED_RED_PORT     13
    #define TEMPERATURE_CHANNEL 1
    #define W1_GPIO3

#elif defined(__BOARD_dimmer)

	#define DEVICE_NAME "DIMMER"
	#define BTN_PORT         2
    #define DIMMER_CHANNEL   0

#endif

#ifndef CFG_SECTOR
	#define CFG_SECTOR          STD_CFG_SECTOR
#endif

#define AP_SSID "SUPLA-ESP8266"

#define SPI_FLASH_SEC_SIZE  4096
#define SERVER_MAXSIZE      100
#define WIFI_SSID_MAXSIZE   32
#define WIFI_PWD_MAXSIZE    64

#define STATE_MAXSIZE       200

#define RECVBUFF_MAXSIZE  1024

#define ACTIVITY_TIMEOUT 10

#if defined(SENSOR_DHT11) || defined(SENSOR_DHT22) || defined(SENSOR_AM2302)
#define DHTSENSOR
#endif

#endif /* SUPLA_ESP_H_ */
