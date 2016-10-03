/*
 ============================================================================
 Name        : supla_esp_gpio.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_GPIO_H_
#define SUPLA_ESP_GPIO_H_

#include "supla_esp.h"

#define RELAY_FLAG_RESET          0x01
#define RELAY_FLAG_RESTORE        0x02
#define RELAY_FLAG_RESTORE_FORCE  0x04
#define RELAY_FLAG_IRQ_LOCKED     0x08

typedef struct {

	uint8 gpio_id;
	uint8 flags;
	uint8 type;
	uint8 step;
	uint8 cycle_counter;
	uint8 cfg_counter;
	uint8 relay_gpio_id;
	uint8 channel;
	uint8 last_state;

	ETSTimer timer;
	unsigned int last_active;

}supla_input_cfg_t;

typedef struct {

	uint8 gpio_id;
	uint8 bind;
	uint8 flags;
	uint8 channel;

	unsigned int last_time;
	ETSTimer timer;

}supla_relay_cfg_t;

extern supla_input_cfg_t supla_input_cfg[INPUT_MAX_COUNT];
extern supla_relay_cfg_t supla_relay_cfg[RELAY_MAX_COUNT];
extern unsigned int supla_esp_gpio_init_time;

void gpio16_output_conf(void);
void gpio16_output_set(uint8 value);
void gpio16_input_conf(void);
uint8 gpio16_input_get(void);
uint8 gpio16_output_get(void);
uint8 gpio__input_get(uint8 port);

void GPIO_ICACHE_FLASH supla_esp_gpio_init(void);

void GPIO_ICACHE_FLASH supla_esp_gpio_state_disconnected(void);
void GPIO_ICACHE_FLASH supla_esp_gpio_state_ipreceived(void);
void GPIO_ICACHE_FLASH supla_esp_gpio_state_connected(void);
void GPIO_ICACHE_FLASH supla_esp_gpio_state_cfgmode(void);

void supla_esp_gpio_hi(int port, char hi);
char supla_esp_gpio_is_hi(int port);


#endif /* SUPLA_ESP_GPIO_H_ */

