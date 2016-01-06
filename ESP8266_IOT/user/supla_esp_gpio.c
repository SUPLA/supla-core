/*
 ============================================================================
 Name        : supla_esp_gpio.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#include <ets_sys.h>
#include <gpio.h>
#include <osapi.h>
#include <mem.h>
#include <os_type.h>
#include "supla_esp.h"
#include "supla_esp_gpio.h"

#include "supla-dev/log.h"
#include "driver/key.h"

#define GPIO_OUTPUT_GET(gpio_no)     ((gpio_output_get()>>gpio_no)&BIT0)

static ETSTimer supla_gpio_timer1;

static struct single_key_param *single_key[1];
static struct keys_param keys;
static char cfg_exit_counter = 1; // 1 for start cfg from user_init


uint32 ICACHE_FLASH_ATTR
gpio_output_get(void)
{
    return GPIO_REG_READ(GPIO_OUT_ADDRESS);
}

void ICACHE_FLASH_ATTR supla_esg_gpio_cfg_pressed(void) {

	if ( supla_esp_cfgmode_started() == 0 ) {

		cfg_exit_counter = 0;

		supla_esp_devconn_stop();
		supla_esp_cfgmode_start();

	};

}

void ICACHE_FLASH_ATTR supla_esg_gpio_manual_pressed(void) {

	if ( supla_esp_cfgmode_started() == 0 ) {

		char hi = supla_esp_gpio_is_hi(RELAY1_PORT) == 1 ? 0 : 1;

		supla_esp_gpio_hi(RELAY1_PORT, hi);
		supla_esp_devconn_on_port_value_changed(RELAY1_PORT, hi);

	} else {

		cfg_exit_counter++;

		if ( cfg_exit_counter > 1 ) {
            system_restart();
		}
	}

}

void ICACHE_FLASH_ATTR
supla_esp_gpio_init(void) {

	ETS_GPIO_INTR_DISABLE();

	GPIO_PORT_INIT;

    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_RED_PORT), GPIO_PIN_INTR_DISABLE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_GREEN_PORT), GPIO_PIN_INTR_DISABLE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_BLUE_PORT), GPIO_PIN_INTR_DISABLE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(RELAY1_PORT), GPIO_PIN_INTR_DISABLE);

	ETS_GPIO_INTR_ENABLE();

	single_key[0] = key_init_single(BTN_PORT, supla_esg_gpio_cfg_pressed, supla_esg_gpio_manual_pressed);

	keys.key_num = 1;
	keys.single_key = single_key;

	key_init(&keys);

}

void ICACHE_FLASH_ATTR
supla_esp_gpio_hi(int port, char hi) {
	GPIO_OUTPUT_SET(GPIO_ID_PIN(port), hi == 1 ? 1 : 0);
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_led_red_on(char on) {
	supla_esp_gpio_hi(LED_RED_PORT, on == 1 ? 1 : 0);
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_led_green_on(char on) {
	supla_esp_gpio_hi(LED_GREEN_PORT, on == 1 ? 1 : 0);
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_led_blue_on(char on) {
	supla_esp_gpio_hi(LED_BLUE_PORT, on == 1 ? 1 : 0);
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_leg_blinking_func(void *timer_arg) {
	supla_esp_gpio_led_blue_on(GPIO_OUTPUT_GET(LED_BLUE_PORT) == 0 ? 1 : 0);
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_led_blue_blinking(char blink) {

	os_timer_disarm(&supla_gpio_timer1);

	supla_esp_gpio_led_red_on(0);
	supla_esp_gpio_led_green_on(0);
	supla_esp_gpio_led_blue_on(0);

	if ( blink == 1 ) {

	  os_timer_setfn(&supla_gpio_timer1, supla_esp_gpio_leg_blinking_func, NULL);
      os_timer_arm (&supla_gpio_timer1, 500, true);

	}

}

char ICACHE_FLASH_ATTR supla_esp_gpio_is_hi(int port) {
	return GPIO_OUTPUT_GET(port) == 1 ? 1 : 0;
}

