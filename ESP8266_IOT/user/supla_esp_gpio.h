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

typedef void (*_supla_esp_gpio_up_event)(int port);

void ICACHE_FLASH_ATTR supla_esp_gpio_init();
void ICACHE_FLASH_ATTR supla_esp_gpio_led_red_on(char on);
void ICACHE_FLASH_ATTR supla_esp_gpio_led_green_on(char on);
void ICACHE_FLASH_ATTR supla_esp_gpio_led_blue_on(char on);
void ICACHE_FLASH_ATTR supla_esp_gpio_led_blue_blinking(char blink);

char ICACHE_FLASH_ATTR supla_esp_gpio_is_hi(int port);

#endif /* SUPLA_ESP_GPIO_H_ */

