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

#include "mqtt_client_ini_settings.h"
#include <string.h>
#include "svrcfg.h"

supla_mqtt_client_ini_settings::supla_mqtt_client_ini_settings(void)
    : supla_mqtt_client_settings() {}

supla_mqtt_client_ini_settings::~supla_mqtt_client_ini_settings(void) {}

const char *supla_mqtt_client_ini_settings::getHost(void) {
  return scfg_string(CFG_MQTT_HOST);
}

int supla_mqtt_client_ini_settings::getPort(void) {
  return scfg_int(CFG_MQTT_PORT);
}

const char *supla_mqtt_client_ini_settings::getUsername(void) {
  return scfg_string(CFG_MQTT_USERNAME);
}

const char *supla_mqtt_client_ini_settings::getPassword(void) {
  return scfg_string(CFG_MQTT_PASSWORD);
}

const char *supla_mqtt_client_ini_settings::getPrefix(void) {
  return scfg_string(CFG_MQTT_PREFIX);
}

bool supla_mqtt_client_ini_settings::isMQTTEnabled(void) {
  return scfg_int(CFG_MQTT_ENABLED) > 0 && getHost() != NULL &&
         strnlen(getHost(), 2) > 0;
}

bool supla_mqtt_client_ini_settings::isSSLEnabled(void) {
  return scfg_int(CFG_MQTT_SSL);
}

void supla_mqtt_client_ini_settings::getClientId(char *clientId, size_t len,
                                                 const char *suffix) {
  const char *_clientId = scfg_string(CFG_MQTT_CLIENTID);
  if (_clientId && strnlen(_clientId, 2) > 0) {
    makeClientId(clientId, len, _clientId, suffix);
  } else {
    supla_mqtt_client_settings::getClientId(clientId, len, suffix);
  }
}
