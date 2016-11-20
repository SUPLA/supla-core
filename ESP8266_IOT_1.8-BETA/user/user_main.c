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
#include <user_interface.h>

#include "supla_esp.h"
#include "supla_esp_cfg.h"
#include "supla_esp_cfgmode.h"
#include "supla_esp_gpio.h"
#include "supla-dev/log.h"

#include "board/supla_esp_board.c"


uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


void user_rf_pre_init(){};


void user_init(void)
{

	 struct rst_info *rtc_info = system_get_rst_info();
	 supla_log(LOG_DEBUG, "RST reason: %i", rtc_info->reason);


     wifi_status_led_uninstall();
     supla_esp_cfg_init();
     supla_esp_gpio_init();

     supla_log(LOG_DEBUG, "Starting %i", system_get_time());


	 #if NOSSL == 1
      supla_log(LOG_DEBUG, "NO SSL!");
	 #endif

     supla_esp_devconn_init();


     #ifdef DS18B20
		 supla_ds18b20_init();
     #endif

     #ifdef DHTSENSOR
		 supla_dht_init();
     #endif

	#ifdef SUPLA_PWM_COUNT
	     supla_esp_pwm_init();
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

	system_print_meminfo();

/*
	if ( supla_esp_state.ltag != 25 ) {
		supla_log(LOG_DEBUG, "Log state reset");
		memset(supla_esp_state.log, 0, 4000);
		supla_esp_state.len = 0;
		supla_esp_state.ltag = 25;
		supla_esp_save_state(1);
	}

	if ( supla_esp_state.len < 0 || supla_esp_state.len > 20 )
		supla_esp_state.len = 0;

	int a;
	for(a=0;a<supla_esp_state.len;a++)
		supla_log(LOG_DEBUG, "%i. %s", a, supla_esp_state.log[a]);
*/

}

