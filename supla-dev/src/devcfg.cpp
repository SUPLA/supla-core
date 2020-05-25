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

#include "devcfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "channel-io.h"
#include "mcp23008.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/tools.h"

char DEVICE_GUID[SUPLA_GUID_SIZE];
char DEVICE_AUTHKEY[SUPLA_AUTHKEY_SIZE];
char SOFTWARE_VERSION[SUPLA_SOFTVER_MAXSIZE] = "1.0.3";

/**
 * Use type names to process supla configuration file
 */
static int decode_function_type(const char *fnc) {
  if (strcasecmp(fnc, "TEMPERATURE") == 0) {
    return SUPLA_CHANNELFNC_THERMOMETER;
  } else if (strcasecmp(fnc, "GATEWAYLOCK") == 0) {
    return SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK;
  } else if (strcasecmp(fnc, "GATE") == 0) {
    return SUPLA_CHANNELFNC_CONTROLLINGTHEGATE;
  } else if (strcasecmp(fnc, "GARAGEDOOR") == 0) {
    return SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR;
  } else if (strcasecmp(fnc, "HUMIDITY") == 0) {
    return SUPLA_CHANNELFNC_HUMIDITY;
  } else if (strcasecmp(fnc, "TEMPERATURE_AND_HUMIDITY") == 0) {
    return SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE;
  } else if (strcasecmp(fnc, "GATEWAYSENSOR") == 0) {
    return SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY;
  } else if (strcasecmp(fnc, "GATESENSOR") == 0) {
    return SUPLA_CHANNELFNC_OPENINGSENSOR_GATE;
  } else if (strcasecmp(fnc, "GARAGE_DOOR_SENSOR") == 0) {
    return SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR;
  } else if (strcasecmp(fnc, "NOLIQUID") == 0) {
    return SUPLA_CHANNELFNC_NOLIQUIDSENSOR;
  } else if (strcasecmp(fnc, "DOORLOCK") == 0) {
    return SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK;
  } else if (strcasecmp(fnc, "DOORLOCKSENSOR") == 0) {
    return SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR;
  } else if (strcasecmp(fnc, "ROLLERSHUTTER") == 0) {
    return SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
  } else if (strcasecmp(fnc, "ROLLERSHUTTERSENSOR") == 0) {
    return SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER;
  } else if (strcasecmp(fnc, "POWERSWITCH") == 0) {
    return SUPLA_CHANNELFNC_POWERSWITCH;
  } else if (strcasecmp(fnc, "LIGHTSWITCH") == 0) {
    return SUPLA_CHANNELFNC_LIGHTSWITCH;
  } else if (strcasecmp(fnc, "DIMMER") == 0) {
    return SUPLA_CHANNELFNC_DIMMER;
  } else if (strcasecmp(fnc, "RGBLIGHTNING") == 0) {
    return SUPLA_CHANNELFNC_RGBLIGHTING;
  } else if (strcasecmp(fnc, "DIMMERANDRGB") == 0) {
    return SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING;
  } else if (strcasecmp(fnc, "DEPTHSENSOR") == 0) {
    return SUPLA_CHANNELFNC_DEPTHSENSOR;
  } else if (strcasecmp(fnc, "DISTANCESENSOR") == 0) {
    return SUPLA_CHANNELFNC_DISTANCESENSOR;
  } else if (strcasecmp(fnc, "WINDOWSENSOR") == 0) {
    return SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW;
  } else if (strcasecmp(fnc, "PRESSURESENSOR") == 0) {
    return SUPLA_CHANNELFNC_PRESSURESENSOR;
  } else if (strcasecmp(fnc, "RAINSENSOR") == 0) {
    return SUPLA_CHANNELFNC_RAINSENSOR;
  } else if (strcasecmp(fnc, "WEIGHTSENSOR") == 0) {
    return SUPLA_CHANNELFNC_WEIGHTSENSOR;
  } else if (strcasecmp(fnc, "STAIRCASETIMER") == 0) {
    return SUPLA_CHANNELFNC_STAIRCASETIMER;
  } else if (strcasecmp(fnc, "IC_ELECTRICITY_METER") == 0) {
    return SUPLA_CHANNELFNC_IC_ELECTRICITY_METER;
  } else if (strcasecmp(fnc, "IC_GAS_METER") == 0) {
    return SUPLA_CHANNELFNC_IC_GAS_METER;
  } else if (strcasecmp(fnc, "IC_WATER_METER") == 0) {
    return SUPLA_CHANNELFNC_IC_WATER_METER;
  } else if (strcasecmp(fnc, "GENERAL") == 0) {
    return SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT;
  } else if (strcasecmp(fnc, "WINDSENSOR") == 0) {
    return SUPLA_CHANNELFNC_WINDSENSOR;
  } else if (strcasecmp(fnc, "MAILSENSOR") == 0) {
    return SUPLA_CHANNELFNC_MAILSENSOR;
  } else if (strcasecmp(fnc, "VALVE_OPENCLOSE") == 0) {
    return SUPLA_CHANNELFNC_VALVE_OPENCLOSE;
  } else
    return SUPLA_CHANNELFNC_NONE;
}

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
  } else if (strcasecmp(type, "VALVE") == 0) {
    return SUPLA_CHANNELTYPE_VALVE_OPENCLOSE;
  };

  return atoi(type);
}
/*
static int decode_channel_driver(const char *type) {
  if (strcasecmp(type, "mcp23008") == 0) {
    return SUPLA_CHANNELDRIVER_MCP23008;
  }
  return 0;
}*/

void devcfg_channel_cfg(const char *section, const char *name,
                        const char *value) {
  const char *sec_name = "CHANNEL_";
  size_t sec_name_len = strlen(sec_name);

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
  } else if (strcasecmp(name, "function") == 0) {
    channelio_set_function(number, decode_function_type(value));
  } else if (strcasecmp(name, "file") == 0 && strlen(value) > 0) {
    channelio_set_filename(number, value);
  } else if (strcasecmp(name, "command") == 0 && strlen(value) > 0) {
    channelio_set_execute(number, value);
  } else if (strcasecmp(name, "command_on") == 0 && strlen(value) > 0) {
    channelio_set_execute_on(number, value);
  } else if (strcasecmp(name, "command_off") == 0 && strlen(value) > 0) {
    channelio_set_execute_off(number, value);
  } else if (strcasecmp(name, "state_topic") == 0 && strlen(value) > 0) {
    channelio_set_mqtt_topic_in(number, value);
  } else if (strcasecmp(name, "command_topic") == 0 && strlen(value) > 0) {
    channelio_set_mqtt_topic_out(number, value);
  } else if (strcasecmp(name, "retain") == 0 && strlen(value) == 1) {
    channelio_set_mqtt_retain(number, atoi(value));
  } else if (strcasecmp(name, "state_tamplate") == 0 && strlen(value) > 0) {
    channelio_set_mqtt_template_in(number, value);
  } else if (strcasecmp(name, "command_template") == 0 && strlen(value) > 0) {
    channelio_set_mqtt_template_out(number, value);
  } else if (strcasecmp(name, "payload_on") == 0 && strlen(value) > 0) {
    channelio_set_payload_on(number, value);
  } else if (strcasecmp(name, "payload_off") == 0 && strlen(value) > 0) {
    channelio_set_payload_off(number, value);
  } else if (strcasecmp(name, "payload_value") == 0 && strlen(value) > 0) {
    channelio_set_payload_value(number, value);
  } else if (strcasecmp(name, "min_interval_sec") == 0 && strlen(value) > 0) {
    channelio_set_interval(number, atoi(value) % 100000);
  } else if (strcasecmp(name, "min_toggle_sec") == 0 && strlen(value) > 0) {
    channelio_set_toggle(number, atoi(value) % 100000);
  } else if (strcasecmp(name, "file_write_check_sec") == 0 &&
             strlen(value) > 0) {
    channelio_set_file_write_check(number, atoi(value) % 100000);
  } else if (strcasecmp(name, "idTemplate") == 0 && strlen(value) > 0) {
    channelio_set_id_template(number, value);
  } else if (strcasecmp(name, "idValue") == 0 && strlen(value) > 0) {
    channelio_set_id_value(number, value);
  }
}

unsigned char devcfg_init(int argc, char *argv[]) {
  memset(DEVICE_GUID, 0, SUPLA_GUID_SIZE);
  memset(DEVICE_AUTHKEY, 0, SUPLA_AUTHKEY_SIZE);

  unsigned char result = 0;

  scfg_set_callback(devcfg_channel_cfg);

  // !!! order is important !!!
  char *s_global = "GLOBAL";
  scfg_add_str_param(s_global, "device_guid_file", "./var/dev_guid");
  scfg_add_str_param(s_global, "alt_cfg", "");
  scfg_add_str_param(s_global, "state_file", "./var/last_state.txt");
  scfg_add_str_param(s_global, "device_name", "SUPLA VIRTUAL DEVICE");

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

  char *s_mqtt = "MQTT";
  scfg_add_str_param(s_mqtt, "host", "");
  scfg_add_int_param(s_mqtt, "port", 1833);
  scfg_add_str_param(s_mqtt, "username", "");
  scfg_add_str_param(s_mqtt, "password", "");
  scfg_add_str_param(s_mqtt, "client_name", "supla-vd-client");

  result = scfg_load(argc, argv, "./supla-virtual-device.cfg");

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

  char *authkey_file = (char *)malloc(len);
  snprintf(authkey_file, len, "%s.authkey", guid_file);

  result = st_read_authkey_from_file(authkey_file, DEVICE_AUTHKEY, 1);

  free(authkey_file);

  return result;
}

void devcfg_free(void) { scfg_free(); }
