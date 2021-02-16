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

#ifndef MQTT_CLIENT_INI_SETTINGS_H_
#define MQTT_CLIENT_INI_SETTINGS_H_

#include "mqtt_client_settings.h"

class supla_mqtt_client_ini_settings : public supla_mqtt_client_settings {
 public:
  supla_mqtt_client_ini_settings(void);
  virtual ~supla_mqtt_client_ini_settings(void);
  virtual const char *getHost(void);
  virtual int getPort(void);
  virtual const char *getUsername(void);
  virtual const char *getPassword(void);
  virtual const char *getPrefix(void);
  virtual bool isMQTTEnabled(void);
  virtual bool isSSLEnabled(void);
  virtual void getClientId(char *clientId, size_t len, const char *suffix);
  virtual int getKeepAlive(void);
};

#endif /*MQTT_CLIENT_INI_SETTINGS_H_*/
