/*
 ============================================================================
 Name        : supla_esp_devconn.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_CLIENT_H_
#define SUPLA_ESP_CLIENT_H_

#include "supla_esp.h"

void ICACHE_FLASH_ATTR supla_esp_devconn_init(void);
void ICACHE_FLASH_ATTR supla_esp_devconn_start(void);
void ICACHE_FLASH_ATTR supla_esp_devconn_stop(void);
char * ICACHE_FLASH_ATTR supla_esp_devconn_laststate(void);
void ICACHE_FLASH_ATTR supla_esp_devconn_on_port_value_changed(int port, char hi);

#endif /* SUPLA_ESP_CLIENT_H_ */
