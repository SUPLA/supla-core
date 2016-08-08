/*
 ============================================================================
 Name        : supla_esp_gpio.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.2
 Copyright   : GPLv2
 ============================================================================
*/

#include <ets_sys.h>
#include <osapi.h>
#include <mem.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include "supla_esp.h"
#include "supla_esp_gpio.h"
#include "supla_esp_cfg.h"

#include "supla-dev/log.h"
#include "driver/key.h"

#define GPIO_OUTPUT_GET(gpio_no)     ((gpio_output_get()>>gpio_no)&BIT0)

#define LED_RED    0x1
#define LED_GREEN  0x2
#define LED_BLUE   0x4

#define STATE_UNKNOWN       0
#define STATE_DISCONNECTED  1
#define STATE_IPRECEIVED    2
#define STATE_CONNECTED     4
#define STATE_CFGMODE       5

static unsigned int supla_esp_gpio_init_time = 0;

static unsigned int relay1_last_time = 2147483647;
static unsigned int relay2_last_time = 2147483647;

static char supla_last_state = STATE_UNKNOWN;
static ETSTimer supla_gpio_timer1;

#if defined(INPUT_PORT1) || defined(INPUT_PORT2)
static ETSTimer supla_gpio_timer2;
static ETSTimer supla_gpio_timer3;
static char input1_last_state = -1;
static char input2_last_state = -1;
#endif

#ifdef RELAY1_ELLOCK_MEM
    static char ellock_input1_state = 1;
#endif

static ETSTimer supla_gpio_timer4;
static ETSTimer supla_gpio_timer5;

static char switch_cfgbtn_state_check = 0;
static char switch_cfgbtn_last_state = -1;

static char switch_cfgbtn_counter = 0;

static struct single_key_param *single_key[3];
static struct keys_param keys;
static char cfg_exit_counter = 1; // 1 for start cfg from user_init

void  supla_esg_gpio_relay_switch(char port);

#ifdef __BOARD_zam_row_01
#include "com/zam/supla_esp_gpio.c"
#endif

void
gpio16_output_conf(void)
{
    WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
                   (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 	// mux configuration for XPD_DCDC to output rtc_gpio0

    WRITE_PERI_REG(RTC_GPIO_CONF,
                   (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

    WRITE_PERI_REG(RTC_GPIO_ENABLE,
                   (READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe) | (uint32)0x1);	//out enable
}

void
gpio16_output_set(uint8 value)
{
    WRITE_PERI_REG(RTC_GPIO_OUT,
                   (READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe) | (uint32)(value & 1));
}

void
gpio16_input_conf(void)
{
    WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
                   (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 	// mux configuration for XPD_DCDC and rtc_gpio0 connection

    WRITE_PERI_REG(RTC_GPIO_CONF,
                   (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

    WRITE_PERI_REG(RTC_GPIO_ENABLE,
                   READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe);	//out disable
}

uint8
gpio16_input_get(void)
{
    return (uint8)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
}

uint8
gpio16_output_get(void)
{
    return (uint8)(READ_PERI_REG(RTC_GPIO_OUT) & 1);
}

uint32
gpio_output_get(void)
{
    return GPIO_REG_READ(GPIO_OUT_ADDRESS);
}

void  supla_esg_gpio_cfg_pressed(void) {

	if ( supla_esp_cfgmode_started() == 0 ) {

		cfg_exit_counter = 0;
		switch_cfgbtn_counter = 0;

		supla_esp_devconn_stop();
		supla_esp_cfgmode_start();

	};

}

void
supla_esp_gpio_enable_input_port(char port) {

    gpio_output_set(0, 0, 0, GPIO_ID_PIN(port));

    gpio_register_set(GPIO_PIN_ADDR(port), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                      | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                      | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(port));
    gpio_pin_intr_state_set(GPIO_ID_PIN(port), GPIO_PIN_INTR_ANYEDGE);

}

#if defined(INPUT_PORT1) || defined(INPUT_PORT2)
void
supla_esp_gpio_check_inputs(void *timer_arg) {

    #ifdef INPUT_PORT1
		char i1 = GPIO_INPUT_GET(GPIO_ID_PIN(INPUT_PORT1));

		if ( i1 != input1_last_state ) {

			char _input1_last_state = input1_last_state;
			input1_last_state = i1;

			#ifdef RELAY1_ELLOCK_MEM

			    if (  i1 == 1 && _input1_last_state == 0 ) {
			    	ellock_input1_state = 1;
				}

			    if ( i1 == 1 ) {
			    	i1 = ellock_input1_state;
			    };

			#endif

			#if defined(__BOARD_rs_module) || defined(__BOARD_rs_module_wroom) || defined(__BOARD_jangoe_rs)
			supla_esp_channel_value_changed(1, i1 == 1 ? 1 : 0);
			#else
			supla_esp_channel_value_changed(2, i1 == 1 ? 1 : 0);
			#endif

		}

    #endif

	#ifdef INPUT_PORT2
		char i2 = GPIO_INPUT_GET(GPIO_ID_PIN(INPUT_PORT2));

		if ( i2 != input2_last_state ) {
			input2_last_state = i2;

			supla_esp_channel_value_changed(3, i2 == 1 ? 1 : 0);
		}
	#endif
}
#endif

#if defined(RELAY1_PORT) || defined(RELAY2_PORT)
char supla_esp_gpio_relay_hi(int port, unsigned int *time, char hi) {

    unsigned int t = system_get_time();
    char result = 0;

    RELAY_BEFORE_CHANGE_STATE;

    //supla_log(LOG_DEBUG, "supla_esp_gpio_relay_hi %i, %i, %i, %i, %i", port, hi, relay1_last_time, t, abs(t-(*time)));


    if ( abs(t-(*time)) >= 100000 ) {

    	supla_esp_gpio_hi(port, hi);
    	os_delay_us(10000);
    	supla_esp_gpio_hi(port, hi);

    	*time = t;
    	result = 1;
    }

	RELAY_AFTER_CHANGE_STATE;

    return result;
}
#endif

#ifdef RELAY1_PORT

char supla_esp_gpio_relay1_hi(char hi) {

	char result = supla_esp_gpio_relay_hi(RELAY1_PORT, &relay1_last_time, hi);

	#ifdef RELAY1_ELLOCK_MEM

		if ( result == 1 && hi == 1 ) {

			ellock_input1_state = 0;
			input1_last_state = -1;

			os_timer_disarm(&supla_gpio_timer3);
			supla_esp_gpio_check_inputs(NULL);
		}

	#endif

	return result;
}


void supla_esg_gpio_relay_switch(char port) {

	char hi = supla_esp_gpio_is_hi(port) == 1 ? 0 : 1;

	supla_log(LOG_DEBUG, "supla_esg_gpio_relay_switch %i, %i", port, hi);

	#ifdef RELAY_STATE_RESTORE

	    if ( RELAY1_PORT == port )
			supla_esp_state.Relay1 = hi;
	    else if ( RELAY2_PORT == port )
	    	supla_esp_state.Relay2 = hi;

		supla_esp_save_state(SAVE_STATE_DELAY);
	#endif

		if ( RELAY1_PORT == port ) {

			if ( supla_esp_gpio_relay1_hi(hi) == 1 ) {
				supla_esp_channel_value_changed(0, hi);
			}

		} else {

			if ( supla_esp_gpio_relay2_hi(hi) == 1 ) {
				supla_esp_channel_value_changed(1, hi);
			}

		}



}

#endif

#ifdef RELAY2_PORT

char supla_esp_gpio_relay2_hi(char hi) {
	return supla_esp_gpio_relay_hi(RELAY2_PORT, &relay2_last_time, hi);
}

#endif


#if defined(DIMMER_CHANNEL)

void supla_esg_gpio_pwm_onoff(void) {

	char pwm_on = supla_esp_pwm_is_on() == 1 ? 0 : 1 ;
	supla_esp_pwm_on(pwm_on);

	supla_esp_channel_rgbw_value_changed(DIMMER_CHANNEL, 0, 0, pwm_on ? 100 : 0);

}

#endif

void supla_esg_gpio_manual_pressed(void) {


	if ( supla_esp_cfgmode_started() == 0 ) {

        #if defined(__BOARD_wifisocket) \
		    || defined(__BOARD_wifisocket_54) \
		    || defined(__BOARD_wifisocket_esp01) \
		    || defined(__BOARD_sonoff) \
		    || defined(__BOARD_sonoff_ds18b20)

		supla_esg_gpio_relay_switch(RELAY1_PORT);

        #elif defined(DIMMER_CHANNEL)

		supla_esg_gpio_pwm_onoff();

        #endif

	} else {

		cfg_exit_counter++;

		if ( cfg_exit_counter > 1
			 && switch_cfgbtn_counter < 10 ) {

            system_restart();
		}
	}

}

#if defined(BUTTON1_PORT) && defined(RELAY1_PORT)
void
supla_esg_gpio_button1_pressed(void) {
	supla_esg_gpio_relay_switch(RELAY1_PORT);
}
#endif

#if defined(BUTTON2_PORT) && defined(RELAY2_PORT)
void
supla_esg_gpio_button2_pressed(void) {
	supla_esg_gpio_relay_switch(RELAY2_PORT);
}
#endif

void
supla_esp_gpio_reset_cfg_counter(void *timer_arg) {
	switch_cfgbtn_counter = 0;
}

void
supla_esp_gpio_check_switch_cfgbtn(void *timer_arg) {

	char v = GPIO_INPUT_GET(GPIO_ID_PIN((int)timer_arg));

	if ( v != switch_cfgbtn_last_state ) {

		if ( switch_cfgbtn_counter == 0 ) {

			os_timer_disarm(&supla_gpio_timer5);
			os_timer_setfn(&supla_gpio_timer5, supla_esp_gpio_reset_cfg_counter, NULL);
			os_timer_arm (&supla_gpio_timer5, 10000, false);

		}

		switch_cfgbtn_counter++;

		supla_log(LOG_DEBUG, "Switch counter: %i", switch_cfgbtn_counter);

		if ( switch_cfgbtn_counter >= 10 ) {
			supla_esg_gpio_cfg_pressed();
		} else {
			supla_esg_gpio_manual_pressed();
		}

		switch_cfgbtn_last_state = v;
	}

	switch_cfgbtn_state_check = 0;

}

LOCAL char
supla_esp_key_intr_handler(uint32 gpio_status) {


    #if defined(INPUT_PORT1) || defined(INPUT_PORT2)

	    char start_timer = 0;

		#if defined(INPUT_PORT1)
			if ( gpio_status & BIT(INPUT_PORT1) ) {

				if ( gpio_status & BIT(INPUT_PORT1) )
				   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(INPUT_PORT1));

				start_timer = 1;
			}
		#endif

		#if defined(INPUT_PORT2)
			if ( gpio_status & BIT(INPUT_PORT2) ) {

				if ( gpio_status & BIT(INPUT_PORT2) )
				   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(INPUT_PORT2));

				start_timer = 1;
			}
		#endif

		if ( start_timer == 1 ) {
			os_timer_disarm(&supla_gpio_timer3);
			os_timer_setfn(&supla_gpio_timer3, supla_esp_gpio_check_inputs, NULL);
			os_timer_arm (&supla_gpio_timer3, 500, false);
		}


    #endif

	char handler_result = 0;

	#if defined(ZAM_INPUT1)

		if ( supla_esp_key_intr_zam_handler(gpio_status, ZAM_INPUT1, supla_esp_cfg.Button1Type) == 1 )
			handler_result = 1;

	#endif

	#if defined(ZAM_INPUT2)

		if ( supla_esp_key_intr_zam_handler(gpio_status, ZAM_INPUT2, supla_esp_cfg.Button2Type) == 1 )
			handler_result = 1;

	#endif

	if ( handler_result == 1 )
		return 1;

	if ( supla_esp_cfg.CfgButtonType == BTN_TYPE_SWITCH ) {
		if ( gpio_status & BIT(CFG_PORT) ) {

			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(CFG_PORT));

			if ( switch_cfgbtn_state_check == 0 ) {
				switch_cfgbtn_state_check = 1;

				os_timer_disarm(&supla_gpio_timer4);
				os_timer_setfn(&supla_gpio_timer4, supla_esp_gpio_check_switch_cfgbtn, (void*)CFG_PORT);
				os_timer_arm (&supla_gpio_timer4, 50, false);
			}

			return 1;
		}
	};


	return 0;
}

void ICACHE_FLASH_ATTR
supla_esp_gpio_init(void) {

	switch_cfgbtn_state_check = 0;
	switch_cfgbtn_last_state = -1;
	switch_cfgbtn_counter = 0;

	#if defined(USE_GPIO3) || defined(UART_SWAP)
		 system_uart_swap ();
	#endif

	ETS_GPIO_INTR_DISABLE();

	GPIO_PORT_INIT;

    #ifdef USE_GPIO16_INPUT
	gpio16_input_conf();
    #endif

	#ifdef USE_GPIO16_OUTPUT
	gpio16_output_conf();
	#endif


    #ifdef USE_GPIO3
	   PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
	   PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0RXD_U);
    #endif

    #ifdef BTN_PULLUP
        #if CFG_PORT == 0
	      PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
        #elif CFG_PORT == 2
          PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
        #elif CFG_PORT == 4
	      PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);
		#elif CFG_PORT == 12
	     PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDI_U);
		#elif CFG_PORT == 13
	      PIN_PULLUP_EN(PERIPHS_IO_MUX_MTCK_U);
		#elif CFG_PORT == 14
	      PIN_PULLUP_EN(PERIPHS_IO_MUX_MTMS_U);
       #endif
    #endif

    #ifdef LED_RED_PORT
      #if LED_RED_PORT != 16
	    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_RED_PORT), GPIO_PIN_INTR_DISABLE);
      #endif
    #endif

    #ifdef LED_GREEN_PORT
    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_GREEN_PORT), GPIO_PIN_INTR_DISABLE);
    #endif

    #ifdef LED_BLUE_PORT
    gpio_pin_intr_state_set(GPIO_ID_PIN(LED_BLUE_PORT), GPIO_PIN_INTR_DISABLE);
    #endif


    #ifdef RELAY1_PORT
    gpio_pin_intr_state_set(GPIO_ID_PIN(RELAY1_PORT), GPIO_PIN_INTR_DISABLE);
    #endif

    #ifdef RELAY2_PORT
    gpio_pin_intr_state_set(GPIO_ID_PIN(RELAY2_PORT), GPIO_PIN_INTR_DISABLE);
    #endif

	#ifdef ZAM_INPUT1
	supla_esp_gpio_enable_input_port(ZAM_INPUT1);
	#endif

	#ifdef ZAM_INPUT2
	supla_esp_gpio_enable_input_port(ZAM_INPUT2);
	#endif

	ETS_GPIO_INTR_ENABLE();

	keys.key_num = 0;

	single_key[0] = key_init_single(CFG_PORT, supla_esg_gpio_cfg_pressed, supla_esg_gpio_manual_pressed);
	keys.key_num++;

    #if defined(BUTTON1_PORT) && defined(RELAY1_PORT)

		single_key[keys.key_num] = key_init_single(BUTTON1_PORT, NULL, supla_esg_gpio_button1_pressed);
		keys.key_num++;

    #endif

	#if defined(BUTTON1_PORT) && defined(RELAY1_PORT)

		single_key[keys.key_num] = key_init_single(BUTTON2_PORT, NULL, supla_esg_gpio_button2_pressed);
		keys.key_num++;

	#endif

	keys.single_key = single_key;
	keys.handler = supla_esp_key_intr_handler;

    #ifdef RELAY_STATE_RESTORE

		struct rst_info *rtc_info = system_get_rst_info();

		#ifdef RELAY1_PORT
			if ( rtc_info->reason == 0 ) {
				   supla_esp_gpio_relay1_hi(supla_esp_state.Relay1);
			}
		#endif

		#ifdef RELAY2_PORT
		    if ( rtc_info->reason == 0 ) {
			   supla_esp_gpio_relay2_hi(supla_esp_state.Relay2);
			}
		#endif

    #elif defined(RESET_RELAY_PORT)

		#ifdef RELAY1_PORT
		   supla_esp_gpio_relay1_hi(RELAY_INIT_VALUE);
		#endif

		#ifdef RELAY2_PORT
		   supla_esp_gpio_relay2_hi(RELAY_INIT_VALUE);
		#endif

    #endif

	key_init(&keys);


    GPIO_PORT_POST_INIT;

    supla_esp_gpio_init_time = system_get_time();

}

void
supla_esp_gpio_hi(int port, char hi) {

	 //supla_log(LOG_DEBUG, "supla_esp_gpio_hi %i, %i", port, hi);

	if ( port == 16 ) {
		gpio16_output_set(hi == 1 ? 1 : 0);
	} else {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(port), hi == 1 ? 1 : 0);
	}

}

void supla_esp_gpio_set_led(char r, char g, char b) {

	os_timer_disarm(&supla_gpio_timer1);

    #if defined(INPUT_PORT1) || defined(INPUT_PORT2)
		os_timer_disarm(&supla_gpio_timer2);
		os_timer_disarm(&supla_gpio_timer3);
    #endif

    #ifdef LED_RED_PORT
		supla_esp_gpio_hi(LED_RED_PORT, r);
    #endif

    #ifdef LED_GREEN_PORT
		supla_esp_gpio_hi(LED_GREEN_PORT, g);
    #endif

	#ifdef LED_BLUE_PORT
		supla_esp_gpio_hi(LED_BLUE_PORT, b);
	#endif
}



#if defined(INPUT_PORT1) || defined(INPUT_PORT2)
void  supla_esp_gpio_init_led(void) {

	ETS_GPIO_INTR_DISABLE();

	#ifdef LED_RED_PORT
		#if LED_RED_PORT != 16
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_RED_PORT));
			gpio_pin_intr_state_set(GPIO_ID_PIN(LED_RED_PORT), GPIO_PIN_INTR_DISABLE);
			gpio_output_set(0, GPIO_ID_PIN(LED_RED_PORT), GPIO_ID_PIN(LED_RED_PORT), 0);
		#endif
	#endif

	#ifdef LED_GREEN_PORT
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_GREEN_PORT));
		gpio_pin_intr_state_set(GPIO_ID_PIN(LED_GREEN_PORT), GPIO_PIN_INTR_DISABLE);
		gpio_output_set(0, GPIO_ID_PIN(LED_GREEN_PORT), GPIO_ID_PIN(LED_GREEN_PORT), 0);
	#endif

	#ifdef LED_BLUE_PORT
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_BLUE_PORT));
		gpio_pin_intr_state_set(GPIO_ID_PIN(LED_BLUE_PORT), GPIO_PIN_INTR_DISABLE);
		gpio_output_set(0, GPIO_ID_PIN(LED_BLUE_PORT), GPIO_ID_PIN(LED_BLUE_PORT), 0);
	#endif

    ETS_GPIO_INTR_ENABLE();

}
#endif


#if defined(LED_RED_PORT) || defined(LED_GREEN_PORT) || defined(LED_BLUE_PORT)
void
supla_esp_gpio_led_blinking_func(void *timer_arg) {

	#ifdef LED_RED_PORT
		if ( (int)timer_arg & LED_RED )
			supla_esp_gpio_hi(LED_RED_PORT, supla_esp_gpio_is_hi(LED_RED_PORT) == 0 ? 1 : 0);
	#endif

	#ifdef LED_GREEN_PORT
		if ( (int)timer_arg & LED_GREEN )
			supla_esp_gpio_hi(LED_GREEN_PORT, supla_esp_gpio_is_hi(LED_GREEN_PORT) == 0 ? 1 : 0);
	#endif

    #ifdef LED_BLUE_PORT
		if ( (int)timer_arg & LED_BLUE )
			supla_esp_gpio_hi(LED_BLUE_PORT, supla_esp_gpio_is_hi(LED_BLUE_PORT) == 0 ? 1 : 0);
    #endif

}

void
supla_esp_gpio_led_blinking(int led, int time) {

	supla_esp_gpio_set_led(0, 0, 0);

	os_timer_setfn(&supla_gpio_timer1, supla_esp_gpio_led_blinking_func, (void*)led);
	os_timer_arm (&supla_gpio_timer1, time, true);

}
#endif

void ICACHE_FLASH_ATTR
supla_esp_gpio_state_disconnected(void) {

	if ( supla_last_state == STATE_DISCONNECTED )
		return;

	supla_last_state = STATE_DISCONNECTED;

	supla_log(LOG_DEBUG, "Disconnected");

    #if defined(INPUT_PORT1) || defined(INPUT_PORT2)
	supla_esp_gpio_init_led();
    #endif


    #if defined(LED_RED_PORT) && defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
	supla_esp_gpio_set_led(1, 0, 0);
    #elif defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
	supla_esp_gpio_led_blinking(LED_GREEN, 500);
    #elif defined(LED_RED_PORT)
	supla_esp_gpio_led_blinking(LED_RED, 2000);
    #endif

}

void ICACHE_FLASH_ATTR
supla_esp_gpio_state_ipreceived(void) {

	if ( supla_last_state == STATE_IPRECEIVED )
		return;

	supla_last_state = STATE_IPRECEIVED;

	supla_log(LOG_DEBUG, "IP Received");

	#if defined(LED_RED_PORT) && defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
	supla_esp_gpio_set_led(0, 0, 1);
	#elif defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
	supla_esp_gpio_led_blinking(LED_GREEN | LED_BLUE, 500);
	#elif defined(LED_RED_PORT)
	supla_esp_gpio_led_blinking(LED_RED, 500);
	#endif
}

#if defined(INPUT_PORT1) || defined(INPUT_PORT2)


void
supla_esp_gpio_enable_inputs(void *timer_arg) {

	supla_esp_gpio_set_led(0, 0, 0);

	input1_last_state = -1;
	input2_last_state = -1;

	ETS_GPIO_INTR_DISABLE();

	#ifdef INPUT_PORT1
		supla_esp_gpio_enable_input_port(INPUT_PORT1);
	#endif

	#ifdef INPUT_PORT2
		supla_esp_gpio_enable_input_port(INPUT_PORT2);
    #endif

	ETS_GPIO_INTR_ENABLE();

	os_timer_disarm(&supla_gpio_timer3);
	supla_esp_gpio_check_inputs(NULL);
}
#endif

void ICACHE_FLASH_ATTR
supla_esp_gpio_state_connected(void) {

	if ( supla_last_state == STATE_CONNECTED )
		return;

	supla_last_state = STATE_CONNECTED;

	supla_log(LOG_DEBUG, "Server connected");


	#if defined(LED_RED_PORT) && defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
	supla_esp_gpio_set_led(0, 1, 0);
	#else
	supla_esp_gpio_set_led(0, 0, 0);
	#endif

	#if defined(INPUT_PORT1) || defined(INPUT_PORT2)
	os_timer_setfn(&supla_gpio_timer2, supla_esp_gpio_enable_inputs, NULL);
    os_timer_arm (&supla_gpio_timer2, 1000, 0);
	#endif
}


void ICACHE_FLASH_ATTR
supla_esp_gpio_state_cfgmode(void) {

	if ( supla_last_state == STATE_CFGMODE )
		return;

	supla_last_state = STATE_CFGMODE;

	#if defined(INPUT_PORT1) || defined(INPUT_PORT2)
    supla_esp_gpio_init_led();
    #endif

	#if defined(LED_RED_PORT) && defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
    supla_esp_gpio_led_blinking(LED_BLUE, 500);
	#elif defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
	supla_esp_gpio_led_blinking(LED_BLUE, 500);
	#elif defined(LED_RED_PORT)
	supla_esp_gpio_led_blinking(LED_RED, 100);
	#endif

}

char  supla_esp_gpio_is_hi(int port) {

	if ( port == 16 ) {
		return gpio16_output_get() == 1 ? 1 : 0;
	}


	return GPIO_OUTPUT_GET(port) == 1 ? 1 : 0;
}

char  supla_esp_gpio_relay_on(int port) {
	return GPIO_OUTPUT_GET(port) == HI_VALUE ? 1 : 0;
}

