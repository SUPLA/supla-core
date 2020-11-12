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

  library_adapter_pub = new supla_mqttc_library_adapter(ini_settings);
  library_adapter_sub = new supla_mqttc_library_adapter(ini_settings);

  publisher = new supla_mqtt_publisher(library_adapter_pub, ini_settings, NULL);
  // subscriber = new supla_mqtt_subscriber(library_adapter_sub, NULL, NULL);
}

supla_mqtt_client_suite::~supla_mqtt_client_suite(void) {
  publisher->stop();
  delete publisher;
  publisher = NULL;

  // publisher->stop();
  // delete subscriber;
  // subscriber = NULL;

  delete library_adapter_pub;
  library_adapter_pub = NULL;

  delete library_adapter_sub;
  library_adapter_sub = NULL;

  delete ini_settings;
  ini_settings = NULL;
}

void supla_mqtt_client_suite::start(void) {
  publisher->start();
  // subscriber->start();
}

void supla_mqtt_client_suite::stop(void) {
  publisher->stop();
  //  subscriber->stop();
}
