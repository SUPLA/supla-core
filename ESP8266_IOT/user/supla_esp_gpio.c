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

#include "supla_esp.h"
#include "supla_esp_gpio.h"


#define GPIO_OUTPUT_GET(gpio_no)     ((gpio_output_get()>>gpio_no)&BIT0)

static ETSTimer supla_gpio_timer1;
static _supla_esp_gpio_up_event __supla_esp_gpio_up_event = NULL;

uint32 ICACHE_FLASH_ATTR
gpio_output_get(void)
{
    return GPIO_REG_READ(GPIO_OUT_ADDRESS);
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_check_status(uint32 gpio_status, uint32 port) {

	if ( gpio_status & BIT(port) ) {

		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(port));

		if ( __supla_esp_gpio_up_event != NULL )
			__supla_esp_gpio_up_event(port);

	}

}

static void supla_esp_key_intr_handler(void *data) {

	uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

	supla_esp_gpio_check_status(gpio_status, CFG_PORT);
	supla_esp_gpio_check_status(gpio_status, MANUAL_PORT);

}

void ICACHE_FLASH_ATTR
supla_esp_gpio_init(_supla_esp_gpio_up_event up_event) {
	
	__supla_esp_gpio_up_event = up_event;

	ETS_GPIO_INTR_ATTACH(supla_esp_key_intr_handler, NULL);
	ETS_GPIO_INTR_DISABLE();

	GPIO_PORT_INIT;
    
    gpio_output_set(0, 0, 0, BIT(CFG_PORT));
    gpio_output_set(0, 0, 0, BIT(MANUAL_PORT));

    supla_esp_gpio_led_red_on(0);
    supla_esp_gpio_led_green_on(0);
    supla_esp_gpio_led_blue_on(0);


    gpio_pin_intr_state_set(GPIO_ID_PIN(CFG_PORT), GPIO_PIN_INTR_NEGEDGE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(MANUAL_PORT), GPIO_PIN_INTR_NEGEDGE);
    ETS_GPIO_INTR_ENABLE();

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

