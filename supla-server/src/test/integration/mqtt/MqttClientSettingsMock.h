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

#ifndef MQTT_CLIENTSETTINGS_MOCK_H_
#define MQTT_CLIENTSETTINGS_MOCK_H_

#include "mqtt_client_ini_settings.h"

class MqttClientSettingsMock : public supla_mqtt_client_ini_settings {
 private:
  bool null_prefix;

 public:
  explicit MqttClientSettingsMock(void);
  ~MqttClientSettingsMock(void);

  virtual const char *getPrefix(void);
  void setNullPrefix(bool empty);
};

#endif /* MQTT_CLIENTSETTINGS_MOCK_H_ */
