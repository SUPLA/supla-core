/*
 ============================================================================
 Name        : supla_esp_gpio.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
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

#define GPIO_OUTPUT_GET(gpio_no)     ((gpio_output_get()>>gpio_no)&BIT0)

#define LED_RED    0x1
#define LED_GREEN  0x2
#define LED_BLUE   0x4

#define STATE_UNKNOWN       0
#define STATE_DISCONNECTED  1
#define STATE_IPRECEIVED    2
#define STATE_CONNECTED     4
#define STATE_CFGMODE       5


#define INPUT_MIN_CYCLE_COUNT   5
#define INPUT_CYCLE_TIME        20

#define CFG_BTN_PRESS_TIME      5000
#define CFG_BTN_PRESS_COUNT     10

supla_input_cfg_t supla_input_cfg[INPUT_MAX_COUNT];
supla_relay_cfg_t supla_relay_cfg[RELAY_MAX_COUNT];

unsigned int supla_esp_gpio_init_time = 0;

static char supla_last_state = STATE_UNKNOWN;
static ETSTimer supla_gpio_timer1;
static ETSTimer supla_gpio_timer2;

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

uint8
gpio__input_get(uint8 port)
{
	if ( port == 16 )
      return gpio16_input_get();

	return GPIO_INPUT_GET(GPIO_ID_PIN(port));
}

void  supla_esg_gpio_start_cfg_mode(void) {

	if ( supla_esp_cfgmode_started() == 0 ) {

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

void supla_esp_gpio_btn_irq_lock(uint8 lock) {

	supla_input_cfg_t *input_cfg;
	int a;

	for(a=0;a<INPUT_MAX_COUNT;a++) {

		input_cfg = &supla_input_cfg[a];

		if ( input_cfg->gpio_id != 255
				&& ( input_cfg->type == INPUT_TYPE_BUTTON
					 || input_cfg->type == INPUT_TYPE_BUTTON_HILO
					 || input_cfg->type == INPUT_TYPE_SWITCH ) )
		     gpio_pin_intr_state_set(GPIO_ID_PIN(input_cfg->gpio_id), lock == 1 ? GPIO_PIN_INTR_DISABLE : GPIO_PIN_INTR_ANYEDGE);

	}


}

char supla_esp_gpio_relay_hi(int port, char hi) {

    unsigned int t = system_get_time();
    unsigned int *time = NULL;
    int a;
    char result = 0;
    char *state = NULL;
    char _hi = hi;

    if ( hi == 255 ) {
    	hi = supla_esp_gpio_relay_is_hi(port) == 1 ? 0 : 1;
    }

    for(a=0;a<RELAY_MAX_COUNT;a++)
    	if ( supla_relay_cfg[a].gpio_id == port ) {

    		time = &supla_relay_cfg[a].last_time;

    		if ( supla_relay_cfg[a].flags & RELAY_FLAG_RESTORE )
    			state = &supla_esp_state.Relay[a];

    		if ( supla_relay_cfg[a].flags &  RELAY_FLAG_HI_LEVEL_TRIGGER )
    			_hi = hi == HI_VALUE ? LO_VALUE : HI_VALUE;

    		if ( hi == 1
    			 && supla_relay_cfg[a].bind != 255
    			 && supla_relay_cfg[a].flags & RELAY_FLAG_TURNOFF_BINDED
    			 && supla_relay_cfg[supla_relay_cfg[a].bind].gpio_id != 255 ) {

    			supla_esp_gpio_relay_hi(supla_relay_cfg[supla_relay_cfg[a].bind].gpio_id, 0);
    			os_delay_us(50000);

    		}

    		break;
    	}


    supla_esp_gpio_btn_irq_lock(1);
    os_delay_us(10);

	#ifdef RELAY_BEFORE_CHANGE_STATE
    RELAY_BEFORE_CHANGE_STATE;
	#endif

    if ( time == NULL
    	 || abs(t-(*time)) >= RELAY_MIN_DELAY ) {

        //supla_log(LOG_DEBUG, "port = %i, hi = %i", port, hi);

    	supla_esp_gpio_hi(port, _hi);
    	os_delay_us(10000);
    	supla_esp_gpio_hi(port, _hi);

    	if ( time != NULL )
        	*time = t;


    	result = 1;
    }

    os_delay_us(10);
    supla_esp_gpio_btn_irq_lock(0);

	#ifdef RELAY_AFTER_CHANGE_STATE
	RELAY_AFTER_CHANGE_STATE;
	#endif

    if ( result == 1
		 && state != NULL ) {

		*state = hi;
		supla_esp_save_state(SAVE_STATE_DELAY);
    }


    return result;
}


#if defined(DIMMER_CHANNEL)
void supla_esg_gpio_pwm_onoff(void) {

	char pwm_on = supla_esp_pwm_is_on() == 1 ? 0 : 1 ;
	supla_esp_pwm_on(pwm_on);

	supla_esp_channel_rgbw_value_changed(DIMMER_CHANNEL, 0, 0, pwm_on ? 100 : 0);

}
#endif


LOCAL void supla_esp_gpio_relay_switch(supla_input_cfg_t *input_cfg, char hi) {

	if (  input_cfg->relay_gpio_id != 255 ) {

		//supla_log(LOG_DEBUG, "RELAY");

		supla_esp_gpio_relay_hi(input_cfg->relay_gpio_id, hi);

		if ( input_cfg->channel != 255 )
			supla_esp_channel_value_changed(input_cfg->channel, supla_esp_gpio_relay_is_hi(input_cfg->relay_gpio_id));

	}

}

LOCAL void
supla_esp_gpio_on_input_active(supla_input_cfg_t *input_cfg) {

	if ( input_cfg->type == INPUT_TYPE_BUTTON_HILO ) {

		//supla_log(LOG_DEBUG, "RELAY HI");
		supla_esp_gpio_relay_switch(input_cfg, 1);

	} else if ( input_cfg->type == INPUT_TYPE_SWITCH ) {

		//supla_log(LOG_DEBUG, "RELAY");
		supla_esp_gpio_relay_switch(input_cfg, 255);


	} else if ( input_cfg->type == INPUT_TYPE_SENSOR
				&&  input_cfg->channel != 255 ) {

		//supla_log(LOG_DEBUG, "active");
		supla_esp_channel_value_changed(input_cfg->channel, 1);

	}

	input_cfg->last_state = 1;
}

LOCAL void
supla_esp_gpio_on_input_inactive(supla_input_cfg_t *input_cfg) {

	if ( input_cfg->type == INPUT_TYPE_BUTTON_HILO ) {

		//supla_log(LOG_DEBUG, "RELAY LO");
		supla_esp_gpio_relay_switch(input_cfg, 0);

	} else if ( input_cfg->type == INPUT_TYPE_BUTTON
		 || input_cfg->type == INPUT_TYPE_SWITCH ) {


		supla_esp_gpio_relay_switch(input_cfg, 255);

        #ifdef DIMMER_CHANNEL
				supla_esg_gpio_pwm_onoff();
		#endif


	} else if ( input_cfg->type == INPUT_TYPE_SENSOR
			    &&  input_cfg->channel != 255 ) {

		//supla_log(LOG_DEBUG, "inactive");
		supla_esp_channel_value_changed(input_cfg->channel, 0);

	}

	input_cfg->last_state = 0;
}

LOCAL void
supla_esp_gpio_input_timer_cb(void *timer_arg) {

	supla_input_cfg_t *input_cfg = (supla_input_cfg_t *)timer_arg;
	uint8 v = gpio__input_get(input_cfg->gpio_id);
	uint8 active = (input_cfg->flags & INPUT_FLAG_PULLUP) ? 0 : 1;

	if ( input_cfg->step == 1 ) {

		if ( v == active ) {

			input_cfg->cycle_counter = 1;

			if ( input_cfg->flags & INPUT_FLAG_CFG_BTN
				 && ( input_cfg->type == INPUT_TYPE_BUTTON
					  || input_cfg->type == INPUT_TYPE_BUTTON_HILO
				      || (system_get_time() - input_cfg->last_active) >= 2000000 ) ) {

				input_cfg->cfg_counter = 0;
				//supla_log(LOG_DEBUG, "RESET cfg_counter = 0");
			}

			input_cfg->step = 2;
			return;
		};

	} else if ( input_cfg->step > 1 ) {

		if ( input_cfg->cycle_counter < 255 )
			input_cfg->cycle_counter++;

		if ( v == active ) {

			if ( input_cfg->step == 3
					&& input_cfg->flags & INPUT_FLAG_CFG_BTN
					&& ( input_cfg->type == INPUT_TYPE_BUTTON
						 || input_cfg->type == INPUT_TYPE_BUTTON_HILO) ) {

				if ( input_cfg->cfg_counter < 255 )
					input_cfg->cfg_counter++;


				if ( (input_cfg->cfg_counter * INPUT_CYCLE_TIME) >= CFG_BTN_PRESS_TIME ) {

					// CFG MODE
					supla_esg_gpio_start_cfg_mode();

					input_cfg->step = 0;
					os_timer_disarm(&input_cfg->timer);
					return;
				}

			}


			if ( input_cfg->step == 2
				 && input_cfg->cycle_counter >= INPUT_MIN_CYCLE_COUNT ) {


				supla_esp_gpio_on_input_active(input_cfg);


				if ( (input_cfg->flags & INPUT_FLAG_CFG_BTN) == 0
						|| input_cfg->type == INPUT_TYPE_SWITCH ) {

					os_timer_disarm(&input_cfg->timer);

				}


				input_cfg->cycle_counter = 1;
				input_cfg->last_active = system_get_time();
				input_cfg->step = 3;

			}


			return;
		}


		if ( input_cfg->step == 3 ) {

			if ( input_cfg->cycle_counter >= INPUT_MIN_CYCLE_COUNT ) {

				if ( input_cfg->flags & INPUT_FLAG_CFG_BTN
						&& input_cfg->type == INPUT_TYPE_SWITCH ) {

					input_cfg->cfg_counter++;

					//supla_log(LOG_DEBUG, "cfg_counter = %i", input_cfg->cfg_counter);

					if ( supla_esp_cfgmode_started() == 0 ) {

						if ( input_cfg->cfg_counter >= CFG_BTN_PRESS_COUNT ) {
							input_cfg->cfg_counter = 0;

							// CFG MODE
							supla_esg_gpio_start_cfg_mode();
						}

					} else if ( input_cfg->cfg_counter > 2 ) {

						//  EXIT CFG MODE
						system_restart();

					}



				}

				if ( input_cfg->flags & INPUT_FLAG_CFG_BTN
						&& supla_esp_cfgmode_started() == 1
						&& ( input_cfg->type == INPUT_TYPE_BUTTON
							 || input_cfg->type == INPUT_TYPE_BUTTON_HILO ) ) {

					// EXIT CFG MODE
					system_restart();

				} else {

					supla_esp_gpio_on_input_inactive(input_cfg);

				}


			} else {
				return;
			}

		}

	}


    input_cfg->step = 0;
    os_timer_disarm(&input_cfg->timer);

}

LOCAL void
supla_esp_gpio_intr_handler(void *params) {

	char a;
	uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	supla_input_cfg_t *input_cfg;

	#ifdef BOARD_INTR_HANDLER
	BOARD_INTR_HANDLER;
	#endif

	//supla_log(LOG_DEBUG, "INTR");

	for(a=0;a<INPUT_MAX_COUNT;a++) {

		input_cfg = &supla_input_cfg[a];

		if ( input_cfg->gpio_id != 255
			 && input_cfg->type != INPUT_TYPE_CUSTOM
			 && gpio_status & BIT(input_cfg->gpio_id) ) {

			//supla_log(LOG_DEBUG, "INTR(b) %i", a);

            gpio_pin_intr_state_set(GPIO_ID_PIN(input_cfg->gpio_id), GPIO_PIN_INTR_DISABLE);
            GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(input_cfg->gpio_id)); // //clear interrupt status

            if ( input_cfg->step == 0
            		|| input_cfg->step == 3 ) {

            	if ( input_cfg->step == 0 )
                	input_cfg->step = 1;

    			os_timer_disarm(&input_cfg->timer);
    			os_timer_setfn(&input_cfg->timer, supla_esp_gpio_input_timer_cb, input_cfg);
    			os_timer_arm (&input_cfg->timer, INPUT_CYCLE_TIME, true);
            }

            gpio_pin_intr_state_set(GPIO_ID_PIN(input_cfg->gpio_id), GPIO_PIN_INTR_ANYEDGE);
		}
	}

}


void GPIO_ICACHE_FLASH
supla_esp_gpio_init(void) {

	char a;

	memset(&supla_input_cfg, 0, sizeof(supla_input_cfg));
	memset(&supla_relay_cfg, 0, sizeof(supla_relay_cfg));

	for (a=0; a<INPUT_MAX_COUNT; a++) {
		supla_input_cfg[a].gpio_id = 255;
		supla_input_cfg[a].relay_gpio_id = 255;
		supla_input_cfg[a].channel = 255;
		supla_input_cfg[a].last_state = 255;
	}

	for (a=0; a<RELAY_MAX_COUNT; a++) {
		supla_relay_cfg[a].gpio_id = 255;
		supla_relay_cfg[a].bind = 255;
		supla_relay_cfg[a].channel = 255;
	}

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

	ETS_GPIO_INTR_ENABLE();

	supla_esp_board_gpio_init();

    ETS_GPIO_INTR_ATTACH(supla_esp_gpio_intr_handler, NULL);

    ETS_GPIO_INTR_DISABLE();

	for (a=0; a<RELAY_MAX_COUNT; a++) {

		if ( supla_relay_cfg[a].gpio_id != 255 ) {

			  //supla_log(LOG_DEBUG, "relay init %i", supla_relay_cfg[a].gpio_id);

			gpio_pin_intr_state_set(GPIO_ID_PIN(supla_relay_cfg[a].gpio_id), GPIO_PIN_INTR_DISABLE);
			supla_relay_cfg[0].last_time = 2147483647;

			if ( supla_relay_cfg[a].flags & RELAY_FLAG_RESTORE_FORCE ) {

				supla_esp_gpio_relay_hi(supla_relay_cfg[a].gpio_id, supla_esp_state.Relay[a]);

			} else if ( supla_relay_cfg[a].flags & RELAY_FLAG_RESTORE ) {

				struct rst_info *rtc_info = system_get_rst_info();

				if ( rtc_info->reason == 0 ) {
					   supla_esp_gpio_relay_hi(supla_relay_cfg[a].gpio_id, supla_esp_state.Relay[a]);
				}

			} else if ( supla_relay_cfg[a].flags & RELAY_FLAG_RESET ) {

				supla_esp_gpio_relay_hi(supla_relay_cfg[a].gpio_id, LO_VALUE);

			}
		}

	}


    for (a=0; a<INPUT_MAX_COUNT; a++)
      if ( supla_input_cfg[a].gpio_id != 255
    		&& supla_input_cfg[a].type != 0) {

    	  //supla_log(LOG_DEBUG, "input init %i", supla_input_cfg[a].gpio_id);

        gpio_output_set(0, 0, 0, GPIO_ID_PIN(supla_input_cfg[a].gpio_id));

        gpio_register_set(GPIO_PIN_ADDR(supla_input_cfg[a].gpio_id), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                          | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                          | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(supla_input_cfg[a].gpio_id));
        gpio_pin_intr_state_set(GPIO_ID_PIN(supla_input_cfg[a].gpio_id), GPIO_PIN_INTR_ANYEDGE);
    }

    ETS_GPIO_INTR_ENABLE();

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
	os_timer_disarm(&supla_gpio_timer2);

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


void GPIO_ICACHE_FLASH supla_esp_gpio_init_led(void) {

	ETS_GPIO_INTR_DISABLE();

	#ifdef LED_RED_PORT
		#if LED_RED_PORT != 16
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_RED_PORT));
			gpio_pin_intr_state_set(GPIO_ID_PIN(LED_RED_PORT), GPIO_PIN_INTR_DISABLE);
			gpio_output_set(0, GPIO_ID_PIN(LED_RED_PORT), GPIO_ID_PIN(LED_RED_PORT), 0);
		#endif
	#endif

	#ifdef LED_GREEN_PORT
		#if LED_GREEN_PORT != 16
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_GREEN_PORT));
			gpio_pin_intr_state_set(GPIO_ID_PIN(LED_GREEN_PORT), GPIO_PIN_INTR_DISABLE);
			gpio_output_set(0, GPIO_ID_PIN(LED_GREEN_PORT), GPIO_ID_PIN(LED_GREEN_PORT), 0);
		#endif
	#endif

	#ifdef LED_BLUE_PORT
		#if LED_BLUE_PORT != 16
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(LED_BLUE_PORT));
			gpio_pin_intr_state_set(GPIO_ID_PIN(LED_BLUE_PORT), GPIO_PIN_INTR_DISABLE);
			gpio_output_set(0, GPIO_ID_PIN(LED_BLUE_PORT), GPIO_ID_PIN(LED_BLUE_PORT), 0);
		#endif
	#endif

    ETS_GPIO_INTR_ENABLE();

}



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

void GPIO_ICACHE_FLASH
supla_esp_gpio_state_disconnected(void) {

	if ( supla_last_state == STATE_DISCONNECTED )
		return;

	supla_last_state = STATE_DISCONNECTED;
	supla_log(LOG_DEBUG, "Disconnected");
	supla_esp_gpio_init_led();


    #if defined(LED_RED_PORT) && defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
	supla_esp_gpio_set_led(1, 0, 0);
    #elif defined(LED_GREEN_PORT) && defined(LED_BLUE_PORT)
	supla_esp_gpio_led_blinking(LED_GREEN, 500);
    #elif defined(LED_RED_PORT)
	supla_esp_gpio_led_blinking(LED_RED, 2000);
    #endif

}

void GPIO_ICACHE_FLASH
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

void GPIO_ICACHE_FLASH
supla_esp_gpio_enable_sensors(void *timer_arg) {

	int a;
	supla_esp_gpio_set_led(0, 0, 0);

	ETS_GPIO_INTR_DISABLE();

	for(a=0;a<INPUT_MAX_COUNT;a++)
		if ( supla_input_cfg[a].gpio_id != 255
			 && supla_input_cfg[a].type == INPUT_TYPE_SENSOR )

		supla_esp_gpio_enable_input_port(supla_input_cfg[a].gpio_id);


	ETS_GPIO_INTR_ENABLE();

}


void GPIO_ICACHE_FLASH
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

	os_timer_setfn(&supla_gpio_timer2, supla_esp_gpio_enable_sensors, NULL);
    os_timer_arm (&supla_gpio_timer2, 1000, 0);
}


void GPIO_ICACHE_FLASH
supla_esp_gpio_state_cfgmode(void) {

	if ( supla_last_state == STATE_CFGMODE )
		return;

	supla_last_state = STATE_CFGMODE;

    supla_esp_gpio_init_led();

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

char supla_esp_gpio_relay_is_hi(int port) {

	char result = supla_esp_gpio_is_hi(port);
	int a;

    for(a=0;a<RELAY_MAX_COUNT;a++)
    	if ( supla_relay_cfg[a].gpio_id == port ) {

    		if ( supla_relay_cfg[a].flags &  RELAY_FLAG_HI_LEVEL_TRIGGER ) {
    			result == HI_VALUE ? LO_VALUE : HI_VALUE;
    		}

    		break;
    	}

    return result;
}

char  supla_esp_gpio_relay_on(int port) {
	return GPIO_OUTPUT_GET(port) == HI_VALUE ? 1 : 0;
}

