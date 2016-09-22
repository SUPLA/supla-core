/*
 ============================================================================
 Name        : supla_dht.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : 2016 GPLv2
 ============================================================================
*/

#ifndef SUPLA_DHTSENSOR_H_
#define SUPLA_DHTSENSOR_H_

#include "supla_esp.h"
#include "supla-dev/proto.h"

#ifdef DHTSENSOR

void ICACHE_FLASH_ATTR supla_dht_init(void);
void supla_get_temp_and_humidity(char value[SUPLA_CHANNELVALUE_SIZE]);
void supla_dht_start(void);

#endif

#endif
