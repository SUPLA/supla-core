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

#ifndef MQTTCLIENTTEST_H_
#define MQTTCLIENTTEST_H_

#include <mqtt_publisher_datasource.h>

#include "doubles/integration/mqtt/MqttClientLibraryAdapterMock.h"
#include "doubles/integration/mqtt/MqttClientSettingsMock.h"
#include "gtest/gtest.h"
#include "mqtt_publisher.h"

namespace testing {

class MqttClientTest : public Test {
 private:
  MqttClientSettingsMock *iniSettings;
  MqttClientLibraryAdapterMock *libraryAdapter;
  supla_mqtt_client_datasource *dataSource;
  supla_mqtt_client *client;

 protected:
  void waitForConnection();
  void waitForPublications(int expectedTopicCount);
  void verify_published(const char *expectedData[], int count);
  void print_expected(void);

  virtual supla_mqtt_client *getClient(void);
  virtual supla_mqtt_client_datasource *getDS(void);
  virtual MqttClientLibraryAdapterMock *getLibAdapter(void);
  virtual MqttClientSettingsMock *getSettings(void);

  virtual supla_mqtt_client *clientInit(
      supla_mqtt_client_library_adapter *library_adapter,
      supla_mqtt_client_settings *settings,
      supla_mqtt_client_datasource *datasource) = 0;
  virtual supla_mqtt_client_datasource *dsInit(
      supla_mqtt_client_settings *settings) = 0;

 public:
  virtual void SetUp();
  virtual void TearDown();
  MqttClientTest();
  virtual ~MqttClientTest();
};

} /* namespace testing */

#endif /* MQTTCLIENTTEST_H_ */
