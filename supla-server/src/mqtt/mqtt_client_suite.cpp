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

#include <mqtt_client_suite.h>
#include <cstddef>
#include "tools.h"

// static
supla_mqtt_client_suite *supla_mqtt_client_suite::_globalInstance = NULL;

// static
supla_mqtt_client_suite *supla_mqtt_client_suite::globalInstance(void) {
  if (_globalInstance == NULL) {
    _globalInstance = new supla_mqtt_client_suite();
  }

  return _globalInstance;
}

// static
void supla_mqtt_client_suite::globalInstanceRelease(void) {
  if (_globalInstance) {
    delete _globalInstance;
    _globalInstance = NULL;
  }
}

supla_mqtt_client_suite::supla_mqtt_client_suite(void) {
  ini_settings = new supla_mqtt_client_ini_settings();
  if (ini_settings->isMQTTEnabled()) {
    library_adapter_pub = new supla_mqttc_library_adapter(ini_settings);
    library_adapter_sub = new supla_mqttc_library_adapter(ini_settings);

    publisher_ds = new supla_mqtt_publisher_datasource(ini_settings);

    publisher = new supla_mqtt_publisher(library_adapter_pub, ini_settings,
                                         publisher_ds);

    value_setter = new supla_mqtt_channel_value_setter(ini_settings);
    subscriber_ds = new supla_mqtt_subscriber_datasource(ini_settings);
    subscriber = new supla_mqtt_subscriber(library_adapter_sub, ini_settings,
                                           subscriber_ds, value_setter);
  } else {
    library_adapter_pub = NULL;
    library_adapter_sub = NULL;
    publisher_ds = NULL;
    publisher = NULL;
    value_setter = NULL;
    subscriber_ds = NULL;
    subscriber = NULL;
  }
}

supla_mqtt_client_suite::~supla_mqtt_client_suite(void) {
  if (publisher) {
    publisher->stop();
    delete publisher;
    publisher = NULL;
  }

  if (subscriber) {
    subscriber->stop();
    delete subscriber;
    subscriber = NULL;
  }

  if (library_adapter_pub) {
    delete library_adapter_pub;
    library_adapter_pub = NULL;
  }

  if (library_adapter_sub) {
    delete library_adapter_sub;
    library_adapter_sub = NULL;
  }

  if (publisher_ds) {
    delete publisher_ds;
    publisher_ds = NULL;
  }

  if (subscriber_ds) {
    delete subscriber_ds;
    subscriber_ds = NULL;
  }

  if (value_setter) {
    delete value_setter;
    value_setter = NULL;
  }

  if (ini_settings) {
    delete ini_settings;
    ini_settings = NULL;
  }
}

void supla_mqtt_client_suite::start(void) {
  if (publisher) {
    publisher->start();
  }

  if (subscriber) {
    subscriber->start();
  }
}

void supla_mqtt_client_suite::stop(void) {
  if (publisher) {
    publisher->stop();
  }

  if (subscriber) {
    subscriber->stop();
  }
}

void supla_mqtt_client_suite::onUserSettingsChanged(int UserID) {
  if (publisher && st_app_terminate == 0) {
    publisher->on_userdata_changed(UserID);
  }
}

void supla_mqtt_client_suite::onChannelsAdded(int UserID, int DeviceID) {
  if (publisher && st_app_terminate == 0) {
    publisher->on_devicedata_changed(UserID, DeviceID);
  }
}

void supla_mqtt_client_suite::onChannelStateChanged(int UserID, int DeviceID,
                                                    int ChannelID) {
  if (publisher && st_app_terminate == 0) {
    publisher->on_channelstate_changed(UserID, DeviceID, ChannelID);
  }
}

void supla_mqtt_client_suite::beforeChannelFunctionChange(int UserID,
                                                          int ChannelID) {}

void supla_mqtt_client_suite::beforeDeviceDelete(int UserID, int DeviceID) {}
