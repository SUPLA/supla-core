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

#include "mqtt_client_settings.h"
#include <string.h>
#include <cstdio>

supla_mqtt_client_settings::supla_mqtt_client_settings(void) {}

supla_mqtt_client_settings::~supla_mqtt_client_settings(void) {}

void supla_mqtt_client_settings::makeClientId(char *clientId, size_t len,
                                              const char *prefix,
                                              const char *suffix) {
  if (clientId && len && prefix) {
    bool s = suffix && strnlen(suffix, 2) > 0;
    snprintf(clientId, len, "%s%s%s", prefix, s ? "-" : "", s ? suffix : "");
  }
}

void supla_mqtt_client_settings::getClientId(char *clientId, size_t len,
                                             const char *suffix) {
  makeClientId(clientId, len, "supla-server", suffix);
}
