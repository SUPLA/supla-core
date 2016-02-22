/*
 ============================================================================
 Name        : supla_ds18b20.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : 2016 GPLv2
 ============================================================================
*/

#ifndef SUPLA_DS18B20_H_
#define SUPLA_DS18B20_H_

#include "supla_esp.h"

void ICACHE_FLASH_ATTR supla_ds18b20_init(void);
void ICACHE_FLASH_ATTR supla_ds18b20_get_temp(double *value);

#endif
