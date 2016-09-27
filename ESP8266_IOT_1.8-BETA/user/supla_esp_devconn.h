/*
 ============================================================================
 Name        : supla_esp_devconn.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.2
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_CLIENT_H_
#define SUPLA_ESP_CLIENT_H_

#include "supla_esp.h"


void DEVCONN_ICACHE_FLASH supla_esp_devconn_init(void);
void DEVCONN_ICACHE_FLASH supla_esp_devconn_start(void);
void DEVCONN_ICACHE_FLASH supla_esp_devconn_stop(void);
char * DEVCONN_ICACHE_FLASH supla_esp_devconn_laststate(void);
void DEVCONN_ICACHE_FLASH supla_esp_channel_value_changed(int channel_number, char v);
void DEVCONN_ICACHE_FLASH supla_esp_devconn_send_channel_values_with_delay(void);

#ifdef TEMPERATURE_CHANNEL
void DEVCONN_ICACHE_FLASH supla_esp_devconn_on_temp_humidity_changed(char humidity);
#endif

#endif /* SUPLA_ESP_CLIENT_H_ */
