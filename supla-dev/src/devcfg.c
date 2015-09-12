/*
 ============================================================================
 Name        : devcfg.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <stdlib.h>
#include <string.h>

#include "devcfg.h"
#include "tools.h"
#include "log.h"
#include "channel-io.h"

char DEVICE_GUID[SUPLA_GUID_SIZE];

void devcfg_channel_cfg(const char* section, const char* name, const char* value) {


	if ( strlen(section) <= 8
		 || strncasecmp(section, "CHANNEL_", 8) != 0 )
		return;

	unsigned char number = atoi(&section[8]);


	if ( strcasecmp(name, "type") == 0 ) {
		channelio_set_type(number, atoi(value));
	}

	if ( strcasecmp(name, "gpio1") == 0 ) {
		channelio_set_gpio1(number, atoi(value) % 255);
	}

	if ( strcasecmp(name, "gpio2") == 0 ) {
		channelio_set_gpio2(number, atoi(value) % 255);
	}

	if ( strcasecmp(name, "w1") == 0  && strlen(value) > 0 ) {
		channelio_set_w1(number, value);
	}

}

unsigned char devcfg_init(int argc, char* argv[]) {

	memset(DEVICE_GUID, 0, SUPLA_GUID_SIZE);
	unsigned char result = 0;

	scfg_set_callback(devcfg_channel_cfg);

	// !!! order is important !!!
	char *s_global = "GLOBAL";
	scfg_add_str_param(s_global, "device_guid_file", "");
	scfg_add_str_param(s_global, "alt_cfg", "");
	scfg_add_str_param(s_global, "state_file", "");
	scfg_add_str_param(s_global, "device_name", "");

	char *s_server = "SERVER";
	scfg_add_str_param(s_server, "host", "");
	scfg_add_int_param(s_server, "tcp_port", 2015);
	scfg_add_int_param(s_server, "ssl_port", 2016);
	scfg_add_bool_param(s_server, "ssl_enabled", 1);

	char *s_location = "LOCATION";

	scfg_add_int_param(s_location, "ID", 0);
	scfg_add_str_param(s_location, "PASSWORD", 0);

	result = scfg_load(argc, argv, "/etc/supla-dev/supla.cfg");

	if ( result == 1
		 && st_file_exists(scfg_string(CFG_ALTCFG_FILE)) == 1 ) {

		result = scfg_load(argc, argv, scfg_string(CFG_ALTCFG_FILE));
	}

	scfg_names_free();

	return result;
}

char devcfg_getdev_guid() {
	return st_read_guid_from_file(scfg_string(CFG_GUID_FILE), DEVICE_GUID, 1);
}

void devcfg_free(void) {
	scfg_free();
}

