/*
 ============================================================================
 Name        : user_main.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/


#include <string.h>
#include <stdio.h>

#include <ip_addr.h>
#include <osapi.h>
#include <mem.h>

#include "supla_esp.h"
#include "supla_esp_cfg.h"
#include "supla_esp_cfgmode.h"
#include "supla_esp_gpio.h"
#include "supla-dev/log.h"

static int gpio_up_event_time = 0;

void supla_esp_gpio_up_event(int port) {

	int time = system_get_time();

	supla_log(LOG_DEBUG, "Time: %i, %i, %i", time, gpio_up_event_time, (time - gpio_up_event_time), abs(time - gpio_up_event_time));
	supla_log(LOG_DEBUG, "Free heap size: %i", system_get_free_heap_size());

	if ( abs(time - gpio_up_event_time) < 500000 )
		return;

	gpio_up_event_time = time;

	if ( port == CFG_PORT ) {

		supla_esp_devconn_stop();
		supla_esp_cfgmode_start();

	} else if ( port == MANUAL_PORT ) {

		char hi = supla_esp_gpio_is_hi(RELAY1_PORT) == 1 ? 0 : 1;

		supla_esp_gpio_hi(RELAY1_PORT, hi);
		supla_esp_devconn_on_port_value_changed(RELAY1_PORT, hi);
	}

}

void user_init(void)
{

     supla_log(LOG_DEBUG, "Starting");

     wifi_status_led_uninstall();

     supla_esp_cfg_init();
     supla_esp_devconn_init();
     supla_esp_gpio_init(supla_esp_gpio_up_event);

     if ( supla_esp_cfg.LocationID == 0
    		 || supla_esp_cfg.LocationPwd[0] == 0
    		 || supla_esp_cfg.Server[0] == 0
    		 || supla_esp_cfg.WIFI_PWD[0] == 0
    		 || supla_esp_cfg.WIFI_SSID[0] == 0 ) {

    	 supla_esp_cfgmode_start();
    	 return;
     }


     supla_esp_devconn_start();

}

