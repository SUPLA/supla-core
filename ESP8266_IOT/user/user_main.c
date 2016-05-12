/*
 ============================================================================
 Name        : user_main.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
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

void user_init(void)
{

     wifi_status_led_uninstall();
     supla_esp_gpio_init();

     supla_log(LOG_DEBUG, "Starting");

     supla_esp_cfg_init();
     supla_esp_devconn_init();

     #ifdef DS18B20
		 supla_ds18b20_init();
     #endif

     #ifdef DHTSENSOR
		 supla_dht_init();
     #endif

     if ( supla_esp_cfg.LocationID == 0
    		 || supla_esp_cfg.LocationPwd[0] == 0
    		 || supla_esp_cfg.Server[0] == 0
    		 || supla_esp_cfg.WIFI_PWD[0] == 0
    		 || supla_esp_cfg.WIFI_SSID[0] == 0 ) {

    	 supla_esp_cfgmode_start();
    	 return;
     }


    #ifdef DS18B20
		supla_ds18b20_start();
    #endif

	#ifdef DHTSENSOR
		supla_dht_start();
	#endif

	supla_esp_devconn_start();


}

