/*
 ============================================================================
 Name        : w1.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : 2016 GPLv2
 ============================================================================
*/

#ifndef SUPLA_W1_H_
#define SUPLA_W1_H_

#include <ets_sys.h>
#include "supla_esp.h"

extern int supla_w1_pin;

void ICACHE_FLASH_ATTR supla_w1_init(void);


#endif
