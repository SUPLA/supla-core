/*
 ============================================================================
 Name        : supla_esp.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_H_
#define SUPLA_ESP_H_

#include "supla-dev/proto.h"

//#define WIFI-SOCKET
#define GATEMODULE

#ifdef WIFISOCKET
#define DEVICE_NAME "SUPLA-SOCKET"
#elif defined(GATEMODULE)
#define DEVICE_NAME "SUPLA-GATE-MODULE"
#endif

#define AP_SSID "SUPLA-ESP8266"

#define CFG_SECTOR          0x3C
#define SPI_FLASH_SEC_SIZE  4096
#define SERVER_MAXSIZE      100
#define WIFI_SSID_MAXSIZE   32
#define WIFI_PWD_MAXSIZE    64

#define STATE_MAXSIZE       200

#define RECVBUFF_MAXSIZE  1024

#define ACTIVITY_TIMEOUT 10

#define LED_GREEN_PORT  12   
#define LED_BLUE_PORT   14   

#define RELAY1_PORT      5
#define BTN_PORT         4

#define CHANNEL_TYPE SUPLA_CHANNELTYPE_RELAY

#ifdef WIFISOCKET

#define LED_RED_PORT    13

#define CHANNEL_FUNCLIST SUPLA_BIT_RELAYFUNC_POWERSWITCH \
                         | SUPLA_BIT_RELAYFUNC_LIGHTSWITCH

#define CHANNEL_DEFAULT_FUNC SUPLA_CHANNELFNC_POWERSWITCH

#elif defined(GATEMODULE)

#define RELAY2_PORT      13

#define CHANNEL_FUNCLIST SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATEWAYLOCK \
                         | SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATE \
                         | SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGARAGEDOOR \
                         | SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEDOORLOCK

#define CHANNEL_DEFAULT_FUNC 0

#endif


#define GPIO_PORT_INIT \
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); \
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5); \
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); \
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); \
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14)



#endif /* SUPLA_ESP_H_ */
