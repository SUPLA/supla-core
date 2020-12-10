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

#include "MqttSubscriberIntegrationTest.h"
#include "log.h"
#include "mqtt_subscriber.h"
#include "mqtt_subscriber_datasource.h"

namespace testing {

MqttSubscriberIntegrationTest::MqttSubscriberIntegrationTest()
    : MqttClientIntegrationTest() {
  value_setter = NULL;
}

MqttSubscriberIntegrationTest::~MqttSubscriberIntegrationTest() {}

void MqttSubscriberIntegrationTest::waitForData(int expectedTopicCount) {
  for (int a = 0; a < 5000; a++) {
    if (getLibAdapter()->subscribed_count() == expectedTopicCount &&
        !getDS()->is_context_open()) {
      return;
    }
    usleep(1000);
  }

  EXPECT_EQ(getLibAdapter()->subscribed_count(), expectedTopicCount);
  EXPECT_TRUE(getDS()->is_context_open());
  ASSERT_TRUE(false);
}

supla_mqtt_client *MqttSubscriberIntegrationTest::clientInit(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource) {
  value_setter = new MqttChannelValueSetterMock(settings);
  return new supla_mqtt_subscriber(library_adapter, settings, datasource, NULL);
}

void MqttSubscriberIntegrationTest::TearDown() {
  if (value_setter != NULL) {
    delete value_setter;
    value_setter = NULL;
  }

  MqttClientIntegrationTest::TearDown();
}

supla_mqtt_client_datasource *MqttSubscriberIntegrationTest::dsInit(
    supla_mqtt_client_settings *settings) {
  return new supla_mqtt_subscriber_datasource(settings);
}

TEST_F(MqttSubscriberIntegrationTest, fullScope) {
  waitForConnection();
  waitForData(1);

  ASSERT_EQ(getLibAdapter()->subscribed_pop().compare("supla/+/channels/+/set"),
            0);
}

} /* namespace testing */
