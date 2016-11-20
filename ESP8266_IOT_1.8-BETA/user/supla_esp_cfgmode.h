/*
 ============================================================================
 Name        : supla_esp_cfgmode.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_CFGMODE_H_
#define SUPLA_ESP_CFGMODE_H_

void ICACHE_FLASH_ATTR supla_esp_cfgmode_start(void);
char ICACHE_FLASH_ATTR supla_esp_cfgmode_started(void);
void ICACHE_FLASH_ATTR supla_esp_channel_value_changed(int channel_number, char v);

extern unsigned int supla_esp_cfgmode_entertime;

#endif /* SUPLA_ESP_CFGMODE_H_ */
