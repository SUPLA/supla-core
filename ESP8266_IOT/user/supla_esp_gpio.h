/*
 ============================================================================
 Name        : supla_esp_gpio.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.2
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_GPIO_H_
#define SUPLA_ESP_GPIO_H_

#include "supla_esp.h"

void ICACHE_FLASH_ATTR supla_esp_gpio_init(void);

void ICACHE_FLASH_ATTR supla_esp_gpio_state_disconnected(void);
void ICACHE_FLASH_ATTR supla_esp_gpio_state_ipreceived(void);
void ICACHE_FLASH_ATTR supla_esp_gpio_state_connected(void);
void ICACHE_FLASH_ATTR supla_esp_gpio_state_cfgmode(void);

void ICACHE_FLASH_ATTR supla_esp_gpio_hi(int port, char hi);
char ICACHE_FLASH_ATTR supla_esp_gpio_is_hi(int port);

#endif /* SUPLA_ESP_GPIO_H_ */

