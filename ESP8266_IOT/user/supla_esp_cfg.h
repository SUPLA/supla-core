/*
 ============================================================================
 Name        : supla_esp_cfg.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef SUPLA_ESP_CFG_H_
#define SUPLA_ESP_CFG_H_

#include <c_types.h>

#include "supla-dev/proto.h"
#include "supla_esp.h"

#define BTN_TYPE_BUTTON       0
#define BTN_TYPE_SWITCH       1

typedef struct {

	char TAG[6];
	char GUID[SUPLA_GUID_SIZE];
	char Server[SERVER_MAXSIZE];
	int LocationID;
    char LocationPwd[SUPLA_LOCATION_PWD_MAXSIZE];

    char WIFI_SSID[WIFI_SSID_MAXSIZE];
    char WIFI_PWD[WIFI_PWD_MAXSIZE];

    char CfgButtonType;
    char Button1Type;
    char Button2Type;

}SuplaEspCfg;

extern SuplaEspCfg supla_esp_cfg;

char ICACHE_FLASH_ATTR supla_esp_write_state(char *message);
char ICACHE_FLASH_ATTR supla_esp_read_state(char *message);

char ICACHE_FLASH_ATTR supla_esp_cfg_init(void);
char ICACHE_FLASH_ATTR supla_esp_cfg_save(SuplaEspCfg *cfg);

#endif /* SUPLA_ESP_CFG_H_ */
