/*
 ============================================================================
 Name        : supla_esp_gpio.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.2
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
#include "supla_esp_cfg.h"

#include "supla-dev/log.h"
#include "driver/key.h"

#define GPIO_OUTPUT_GET(gpio_no)     ((gpio_output_get()>>gpio_no)&BIT0)

static ETSTimer supla_gpio_timer1;

#if defined(GATEMODULE) || defined(RSMODULE)
static ETSTimer supla_gpio_timer2;
static ETSTimer supla_gpio_timer3;
static char input1_last_state = -1;
static char input2_last_state = -1;
#endif

static ETSTimer supla_gpio_timer4;
static ETSTimer supla_gpio_timer5;

static char switch_btn_state_check = 0;
static char switch_btn_last_state = -1;

static char switch_btn_counter = 0;

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
		switch_btn_counter = 0;

		supla_esp_devconn_stop();
		supla_esp_cfgmode_start();

	};

}

void ICACHE_FLASH_ATTR supla_esg_gpio_manual_pressed(void) {

	if ( supla_esp_cfgmode_started() == 0 ) {

        #ifdef WIFISOCKET
		char hi = supla_esp_gpio_is_hi(RELAY1_PORT) == 1 ? 0 : 1;

		supla_esp_gpio_hi(RELAY1_PORT, hi);
		supla_esp_channel_value_changed(0, hi);

        #endif

	} else {

		cfg_exit_counter++;

		if ( cfg_exit_counter > 1
			 && switch_btn_counter < 10 ) {

            system_restart();
		}
	}

}

#if defined(GATEMODULE) || defined(RSMODULE)
void ICACHE_FLASH_ATTR
supla_esp_gpio_check_inputs(void *timer_arg) {

	char i1 = GPIO_INPUT_GET(GPIO_ID_PIN(INPUT_PORT1));

    #ifdef GATEMODULE
	char i2 = GPIO_INPUT_GET(GPIO_ID_PIN(INPUT_PORT2));
    #endif

	if ( i1 != input1_last_state ) {
		input1_last_state = i1;

        #ifdef RSMODULE
		supla_esp_channel_value_changed(1, i1 == 1 ? 1 : 0);
        #else
		supla_esp_channel_value_changed(2, i1 == 1 ? 1 : 0);
        #endif

	}

    #ifdef GATEMODULE
	if ( i2 != input2_last_state ) {
		input2_last_state = i2;

		supla_esp_channel_value_changed(3, i2 == 1 ? 1 : 0);
	}
    #endif

}
#endif


void ICACHE_FLASH_ATTR
supla_esp_gpio_reset_cfg_counter(void *timer_arg) {
	switch_btn_counter = 0;
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_check_switch_btn(void *timer_arg) {

	char v = GPIO_INPUT_GET(GPIO_ID_PIN(BTN_PORT));

	if ( v != switch_btn_last_state ) {

		if ( switch_btn_counter == 0 ) {

			os_timer_disarm(&supla_gpio_timer5);
			os_timer_setfn(&supla_gpio_timer5, supla_esp_gpio_reset_cfg_counter, NULL);
			os_timer_arm (&supla_gpio_timer5, 10000, false);

		}

		switch_btn_counter++;

		if ( switch_btn_counter >= 10 ) {
			supla_esg_gpio_cfg_pressed();
		} else {
			supla_esg_gpio_manual_pressed();
		}

		switch_btn_last_state = v;
	}

	switch_btn_state_check = 0;

}

LOCAL char
supla_esp_key_intr_handler(uint32 gpio_status) {


    #if defined(GATEMODULE) || defined(RSMODULE)
		if ( gpio_status & BIT(INPUT_PORT1)
			 #ifdef GATEMODULE
			 || gpio_status & BIT(INPUT_PORT2)
			 #endif
			) {

			if ( gpio_status & BIT(INPUT_PORT1) )
			   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(INPUT_PORT1));

			#ifdef GATEMODULE
			if ( gpio_status & BIT(INPUT_PORT2) )
			   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(INPUT_PORT2));
			#endif

			os_timer_disarm(&supla_gpio_timer3);
			os_timer_setfn(&supla_gpio_timer3, supla_esp_gpio_check_inputs, NULL);
			os_timer_arm (&supla_gpio_timer3, 500, false);

		}
    #endif

	if ( supla_esp_cfg.ButtonType == BTN_TYPE_SWITCH ) {
		if ( gpio_status & BIT(BTN_PORT) ) {

			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(BTN_PORT));

			if ( switch_btn_state_check == 0 ) {
				switch_btn_state_check = 1;

				os_timer_disarm(&supla_gpio_timer4);
				os_timer_setfn(&supla_gpio_timer4, supla_esp_gpio_check_switch_btn, NULL);
				os_timer_arm (&supla_gpio_timer4, 200, false);
			}

			return 1;
		}
	}


	return 0;
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_init(void) {

	switch_btn_state_check = 0;
	switch_btn_last_state = -1;
	switch_btn_counter = 0;

	ETS_GPIO_INTR_DISABLE();

	GPIO_PORT_INIT;

    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_GREEN_PORT), GPIO_PIN_INTR_DISABLE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_BLUE_PORT), GPIO_PIN_INTR_DISABLE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(RELAY1_PORT), GPIO_PIN_INTR_DISABLE);

    #ifdef WIFISOCKET
    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_RED_PORT), GPIO_PIN_INTR_DISABLE);
    #elif defined(GATEMODULE) || defined(RSMODULE)
    gpio_pin_intr_state_set(GPIO_ID_PIN(RELAY2_PORT), GPIO_PIN_INTR_DISABLE);
    #endif

	ETS_GPIO_INTR_ENABLE();

	single_key[0] = key_init_single(BTN_PORT, supla_esg_gpio_cfg_pressed, supla_esg_gpio_manual_pressed);

	keys.key_num = 1;
	keys.single_key = single_key;
	keys.handler = supla_esp_key_intr_handler;

    #if defined(GATEMODULE) || defined(RSMODULE)
	supla_esp_gpio_hi(RELAY1_PORT, 0);
	supla_esp_gpio_hi(RELAY2_PORT, 0);
    #endif

	key_init(&keys);

}

void ICACHE_FLASH_ATTR
supla_esp_gpio_hi(int port, char hi) {
	GPIO_OUTPUT_SET(GPIO_ID_PIN(port), hi == 1 ? 1 : 0);
}

void ICACHE_FLASH_ATTR supla_esp_gpio_set_rgb(char r, char g, char b) {

	os_timer_disarm(&supla_gpio_timer1);

    #if defined(GATEMODULE) || defined(RSMODULE)
	os_timer_disarm(&supla_gpio_timer2);
	os_timer_disarm(&supla_gpio_timer3);
    #elif defined(WIFISOCKET)
	supla_esp_gpio_hi(LED_RED_PORT, r);
    #endif

	supla_esp_gpio_hi(LED_GREEN_PORT, g);
	supla_esp_gpio_hi(LED_BLUE_PORT, b);
}

#if defined(GATEMODULE) || defined(RSMODULE)
void ICACHE_FLASH_ATTR supla_esp_gpio_init_led(void) {

	ETS_GPIO_INTR_DISABLE();

	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_GREEN_PORT));
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_BLUE_PORT));

    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_GREEN_PORT), GPIO_PIN_INTR_DISABLE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_BLUE_PORT), GPIO_PIN_INTR_DISABLE);

	gpio_output_set(0, GPIO_ID_PIN(LED_GREEN_PORT), GPIO_ID_PIN(LED_GREEN_PORT), 0);
	gpio_output_set(0, GPIO_ID_PIN(LED_BLUE_PORT), GPIO_ID_PIN(LED_BLUE_PORT), 0);

    ETS_GPIO_INTR_ENABLE();

}
#endif

void ICACHE_FLASH_ATTR supla_esp_gpio_state_disconnected(void) {

    #if defined(GATEMODULE) || defined(RSMODULE)
	supla_esp_gpio_init_led();
    #endif

	supla_esp_gpio_set_rgb(1, 0, 0);

}

void ICACHE_FLASH_ATTR supla_esp_gpio_state_ipreceived(void) {

	supla_esp_gpio_set_rgb(0, 0, 1);

}

#if defined(GATEMODULE) || defined(RSMODULE)

void ICACHE_FLASH_ATTR
supla_esp_gpio_enable_input_port(char port) {

    gpio_output_set(0, 0, 0, GPIO_ID_PIN(port));

    gpio_register_set(GPIO_PIN_ADDR(port), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                      | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                      | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(port));
    gpio_pin_intr_state_set(GPIO_ID_PIN(port), GPIO_PIN_INTR_ANYEDGE);

}

void ICACHE_FLASH_ATTR
supla_esp_gpio_enable_inputs(void *timer_arg) {

	supla_esp_gpio_set_rgb(0, 0, 0);

	input1_last_state = -1;
	input2_last_state = -1;

	ETS_GPIO_INTR_DISABLE();

	supla_esp_gpio_enable_input_port(INPUT_PORT1);

	#ifdef GATEMODULE
	supla_esp_gpio_enable_input_port(INPUT_PORT2);
    #endif

	ETS_GPIO_INTR_ENABLE();

	os_timer_disarm(&supla_gpio_timer3);
	supla_esp_gpio_check_inputs(NULL);
}
#endif

void ICACHE_FLASH_ATTR supla_esp_gpio_state_connected(void) {

	supla_esp_gpio_set_rgb(0, 1, 0);

	#if defined(GATEMODULE) || defined(RSMODULE)
	os_timer_setfn(&supla_gpio_timer2, supla_esp_gpio_enable_inputs, NULL);
    os_timer_arm (&supla_gpio_timer2, 1000, 0);
	#endif
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_led_blinking_func(void *timer_arg) {
	supla_esp_gpio_hi(LED_BLUE_PORT, GPIO_OUTPUT_GET(LED_BLUE_PORT) == 0 ? 1 : 0);
}

void ICACHE_FLASH_ATTR supla_esp_gpio_state_cfgmode(void) {

	#if defined(GATEMODULE) || defined(RSMODULE)
    supla_esp_gpio_init_led();
    #endif

	supla_esp_gpio_set_rgb(0, 0, 0);

	os_timer_setfn(&supla_gpio_timer1, supla_esp_gpio_led_blinking_func, NULL);
    os_timer_arm (&supla_gpio_timer1, 500, true);


}

char ICACHE_FLASH_ATTR supla_esp_gpio_is_hi(int port) {
	return GPIO_OUTPUT_GET(port) == 1 ? 1 : 0;
}

