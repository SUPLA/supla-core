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

#ifndef MQTT_CLIENT_SUITE_H_
#define MQTT_CLIENT_SUITE_H_

#include "mqtt_client_ini_settings.h"
#include "mqtt_publisher.h"
#include "mqtt_publisher_datasource.h"
#include "mqtt_subscriber.h"
#include "mqttc_library_adapter.h"

class supla_mqtt_client_suite {
 private:
  static supla_mqtt_client_suite *_globalInstance;

  supla_mqtt_client_ini_settings *ini_settings;

  supla_mqttc_library_adapter *library_adapter_pub;
  supla_mqttc_library_adapter *library_adapter_sub;

  supla_mqtt_publisher_datasource *publisher_ds;

  supla_mqtt_publisher *publisher;
  supla_mqtt_subscriber *subscriber;

 public:
  static supla_mqtt_client_suite *globalInstance(void);
  static void globalInstanceRelease(void);
  supla_mqtt_client_suite(void);
  virtual ~supla_mqtt_client_suite(void);
  void start(void);
  void stop(void);
  void onUserSettingsChanged(int UserID);
  void onChannelsAdded(int UserID, int DeviceID);
  void onChannelStateChanged(int UserID, int DeviceID, int ChannelID);
};

#endif /*MQTT_CLIENT_SUITE_H_*/
