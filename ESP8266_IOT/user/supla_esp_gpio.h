/*
 ============================================================================
 Name        : supla_esp_gpio.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_GPIO_H_
#define SUPLA_ESP_GPIO_H_


void ICACHE_FLASH_ATTR supla_esp_gpio_init(void);
void ICACHE_FLASH_ATTR supla_esp_gpio_led_red_on(char on);
void ICACHE_FLASH_ATTR supla_esp_gpio_led_green_on(char on);
void ICACHE_FLASH_ATTR supla_esp_gpio_led_blue_on(char on);
void ICACHE_FLASH_ATTR supla_esp_gpio_led_blue_blinking(char blink);

void ICACHE_FLASH_ATTR supla_esp_gpio_hi(int port, char hi);
char ICACHE_FLASH_ATTR supla_esp_gpio_is_hi(int port);

#endif /* SUPLA_ESP_GPIO_H_ */

