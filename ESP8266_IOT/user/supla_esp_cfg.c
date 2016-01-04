/*
 ============================================================================
 Name        : supla_esp_cfg.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#include <string.h>
#include <stdio.h>

#include <ip_addr.h>
#include <user_interface.h>
#include <espconn.h>
#include <spi_flash.h>
#include <osapi.h>
#include <mem.h>


#include "supla_esp_cfg.h"
#include "supla-dev/log.h"

SuplaEspCfg supla_esp_cfg;

char ICACHE_FLASH_ATTR
supla_esp_cfg_save(SuplaEspCfg *cfg) {

	spi_flash_erase_sector(CFG_SECTOR);

	if ( SPI_FLASH_RESULT_OK == spi_flash_write(CFG_SECTOR * SPI_FLASH_SEC_SIZE, (uint32*)cfg, sizeof(SuplaEspCfg)) ) {
		supla_log(LOG_DEBUG, "CFG WRITE SUCCESS");
		return 1;
	}

	supla_log(LOG_DEBUG, "CFG WRITE FAIL!");
	return 0;
}

char ICACHE_FLASH_ATTR
supla_esp_cfg_init(void) {

	char TAG[6] = {'S','U','P','L','A', 1};
	char mac[6];
	int a;


	if ( SPI_FLASH_RESULT_OK == spi_flash_read(CFG_SECTOR * SPI_FLASH_SEC_SIZE, (uint32*)&supla_esp_cfg, sizeof(SuplaEspCfg)) ) {
	   if ( memcmp(supla_esp_cfg.TAG, TAG, 6) == 0 ) {

		   supla_log(LOG_DEBUG, "CFG READ SUCCESS!");
		   return 1;
	   }
	}

	memcpy(supla_esp_cfg.TAG, TAG, 6);
	supla_esp_cfg.Server[0] = 0;
	supla_esp_cfg.LocationID = 0;
	supla_esp_cfg.LocationPwd[0] = 0;
	supla_esp_cfg.WIFI_PWD[0] = 0;
	supla_esp_cfg.WIFI_SSID[0] = 0;

	os_get_random(supla_esp_cfg.GUID, SUPLA_GUID_SIZE);

	if ( SUPLA_GUID_SIZE >= 6 ) {
		wifi_get_macaddr(STATION_IF, mac);

		for(a=0;a<6;a++)
			supla_esp_cfg.GUID[a] = (supla_esp_cfg.GUID[a] * mac[a]) % 255;
	}

	if ( SUPLA_GUID_SIZE >=12 ) {
		wifi_get_macaddr(SOFTAP_IF, mac);

		for(a=0;a<6;a++)
			supla_esp_cfg.GUID[a+6] = ( supla_esp_cfg.GUID[a+6] * mac[a] ) % 255;
	}

	for(a=0;a<SUPLA_GUID_SIZE;a++) {
		supla_esp_cfg.GUID[a]= (supla_esp_cfg.GUID[a] + system_get_time() + spi_flash_get_id() + system_get_chip_id() + system_get_rtc_time()) % 255;
	}

	if ( supla_esp_cfg_save(&supla_esp_cfg) == 1 ) {
		return 1;
	}

	return 0;
}
