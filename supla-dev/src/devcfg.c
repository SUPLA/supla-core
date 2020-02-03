/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>

#include "channel-io.h"
#include "devcfg.h"
#include "log.h"
#include "mcp23008.h"
#include "tools.h"

char DEVICE_GUID[SUPLA_GUID_SIZE];
char DEVICE_AUTHKEY[SUPLA_AUTHKEY_SIZE];

/**
 * Use type names to process supla configuration file
 */
static int decode_channel_type(const char *type) {
  if (strcasecmp(type, "SENSORNO") == 0) {
    return SUPLA_CHANNELTYPE_SENSORNO;
  } else if (strcasecmp(type, "SENSORNC") == 0) {
    return SUPLA_CHANNELTYPE_SENSORNC;
  } else if (strcasecmp(type, "RELAYHFD4") == 0) {
    return SUPLA_CHANNELTYPE_RELAYHFD4;
  } else if (strcasecmp(type, "RELAYG5LA1A") == 0) {
    return SUPLA_CHANNELTYPE_RELAYG5LA1A;
  } else if (strcasecmp(type, "2XRELAYG5LA1A") == 0) {
    return SUPLA_CHANNELTYPE_2XRELAYG5LA1A;
  } else if (strcasecmp(type, "RELAY") == 0) {
    return SUPLA_CHANNELTYPE_RELAY;
  } else if (strcasecmp(type, "THERMOMETERDS18B20") == 0) {
    return SUPLA_CHANNELTYPE_THERMOMETERDS18B20;
  } else if (strcasecmp(type, "DHT11") == 0) {
    return SUPLA_CHANNELTYPE_DHT11;
  } else if (strcasecmp(type, "DHT22") == 0) {
    return SUPLA_CHANNELTYPE_DHT22;
  } else if (strcasecmp(type, "AM2302") == 0) {
    return SUPLA_CHANNELTYPE_AM2302;
  } else if (strcasecmp(type, "DIMMER") == 0) {
    return SUPLA_CHANNELTYPE_DIMMER;
  } else if (strcasecmp(type, "RGBLEDCONTROLLER") == 0) {
    return SUPLA_CHANNELTYPE_RGBLEDCONTROLLER;
  } else if (strcasecmp(type, "DIMMERANDRGBLED") == 0) {
    return SUPLA_CHANNELTYPE_DIMMERANDRGBLED;
  } else if (strcasecmp(type, "HUMIDITYSENSOR") == 0) {
    return SUPLA_CHANNELTYPE_HUMIDITYSENSOR;
  }

  return atoi(type);
}

static int decode_channel_driver(const char *type) {
  if (strcasecmp(type, "mcp23008") == 0) {
    return SUPLA_CHANNELDRIVER_MCP23008;
  }
  return 0;
}

void devcfg_channel_cfg(const char *section, const char *name,
                        const char *value) {
  const char *sec_name = "CHANNEL_";
  int sec_name_len = strlen(sec_name);

  if (strlen(section) <= sec_name_len ||
      strncasecmp(section, sec_name, sec_name_len) != 0)
    return;

  if (strlen(value) == 0) {
    supla_log(LOG_ERR, "Empty value in configuration file for key: %s", name);
    return;
  }

  unsigned char number = atoi(&section[sec_name_len]);

  if (strcasecmp(name, "type") == 0) {
    channelio_set_type(number, decode_channel_type(value));
  } else if (strcasecmp(name, "gpio1") == 0) {
    channelio_set_gpio1(number, atoi(value) % 255);
  } else if (strcasecmp(name, "gpio2") == 0) {
    channelio_set_gpio2(number, atoi(value) % 255);
  } else if (strcasecmp(name, "bistable") == 0) {
    channelio_set_bistable_flag(number, atoi(value) == 1 ? 1 : 0);
  } else if (strcasecmp(name, "w1") == 0 && strlen(value) > 0) {
    channelio_set_w1(number, value);
  } else if (strcasecmp(name, "driver") == 0) {
    channelio_set_mcp23008_driver(number, decode_channel_driver(value));
  } else if (strcasecmp(name, "mcp_addr") == 0) {
    channelio_set_mcp23008_addr(number, strtol(value, NULL, 16));
  } else if (strcasecmp(name, "mcp_reset") == 0) {
    channelio_set_mcp23008_reset(number, atoi(value) % 255);
  } else if (strcasecmp(name, "mcp_gpio_dir") == 0) {
    channelio_set_mcp23008_gpio_dir(number, atoi(value) % 255);
  } else if (strcasecmp(name, "mcp_gpio_val") == 0) {
    channelio_set_mcp23008_gpio_val(number, atoi(value) % 255);
  } else if (strcasecmp(name, "mcp_gpio_port") == 0) {
    channelio_set_mcp23008_gpio_port(number, atoi(value) % MCP23008_MAX_GPIO);
  }
}

unsigned char devcfg_init(int argc, char *argv[]) {
  memset(DEVICE_GUID, 0, SUPLA_GUID_SIZE);
  memset(DEVICE_AUTHKEY, 0, SUPLA_AUTHKEY_SIZE);

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
  scfg_add_int_param(s_server, "protocol_version", SUPLA_PROTO_VERSION);

  char *s_location = "LOCATION";

  scfg_add_int_param(s_location, "ID", 0);
  scfg_add_str_param(s_location, "PASSWORD", 0);

  char *s_auth = "AUTH";
  scfg_add_str_param(s_auth, "email", "");

  result = scfg_load(argc, argv, "/etc/supla-dev/supla.cfg");

  if (result == 1 && st_file_exists(scfg_string(CFG_ALTCFG_FILE)) == 1) {
    result = scfg_load(argc, argv, scfg_string(CFG_ALTCFG_FILE));
  }

  scfg_names_free();

  return result;
}

char devcfg_getdev_guid() {
  return st_read_guid_from_file(scfg_string(CFG_GUID_FILE), DEVICE_GUID, 1);
}

char devcfg_getdev_authkey() {
  char *guid_file = scfg_string(CFG_GUID_FILE);

  if (guid_file == NULL || strnlen(guid_file, 1024) == 0) return 0;

  int len = strnlen(guid_file, 1024) + 1;
  char result = 0;

  char *authkey_file = malloc(len);
  snprintf(authkey_file, len, "%s.authkey", guid_file);

  result = st_read_authkey_from_file(authkey_file, DEVICE_AUTHKEY, 1);

  free(authkey_file);

  return result;
}

void devcfg_free(void) { scfg_free(); }
