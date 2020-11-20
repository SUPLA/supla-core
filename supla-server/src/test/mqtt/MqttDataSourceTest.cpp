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

#include "MqttDataSourceTest.h"

namespace testing {

MqttDataSourceTest::MqttDataSourceTest(void) {}
MqttDataSourceTest::~MqttDataSourceTest(void) {}

void MqttDataSourceTest::SetUp() {
  iniSettings = new supla_mqtt_client_ini_settings();
  ds = new MqttDataSourceMock(iniSettings);
}

void MqttDataSourceTest::TearDown() {
  delete ds;
  delete iniSettings;
}

bool MqttDataSourceTest::dataExists(void) {
  char *topic_name = NULL;
  void *message = NULL;
  size_t message_size = 0;

  bool result = ds->fetch(&topic_name, &message, &message_size);

  if (message) {
    free(message);
  }

  if (topic_name) {
    free(topic_name);
  }

  return result;
}

bool MqttDataSourceTest::popMessage(const char *scope, int user_id,
                                    int device_id, int channel_id, int idx) {
  char *topic_name = NULL;
  void *message = NULL;
  size_t message_size = 0;
  int pass = 0;

  char _expected_message[100];
  char _expected_topic_name[100];

  if (!ds->fetch(&topic_name, &message, &message_size)) {
    EXPECT_TRUE(false);
    return false;
  }

  snprintf(_expected_message, sizeof(_expected_message), "%s%i", scope, idx);

  EXPECT_TRUE(topic_name != NULL);
  EXPECT_TRUE(message != NULL);
  EXPECT_EQ(message_size, strlen(_expected_message) + 1);

  if (message) {
    if (memcmp(message, _expected_message, message_size) == 0) {
      pass++;
    } else {
      EXPECT_TRUE(false);
    }

    free(message);
    message = NULL;
  }

  if (topic_name) {
    snprintf(_expected_topic_name, sizeof(_expected_topic_name),
             "/user/%i/device/%i/channel/%i/%i", user_id, device_id, channel_id,
             idx);
    if (strcmp(topic_name, _expected_topic_name) == 0) {
      pass++;
    } else {
      EXPECT_TRUE(false);
    }

    free(topic_name);
    topic_name = NULL;
  }

  return pass == 2;
}

TEST_F(MqttDataSourceTest, empty) {
  char *topic_name = NULL;
  ASSERT_EQ(ds->openCount(), 0);
  ASSERT_EQ(ds->closeCount(), 0);
  ASSERT_FALSE(ds->fetch(&topic_name));
  ASSERT_TRUE(topic_name == NULL);
  ASSERT_EQ(ds->openCount(), 0);
  ASSERT_EQ(ds->closeCount(), 0);
}

TEST_F(MqttDataSourceTest, onBrokerConnected) {
  ASSERT_EQ(ds->openCount(), 0);
  ASSERT_EQ(ds->closeCount(), 0);

  ds->on_channelvalue_changed(55, 1, 123);
  ds->on_devicedata_changed(56, 2);
  ds->on_userdata_changed(57);

  ds->on_broker_connected();

  ds->on_userdata_changed(58);
  ds->on_devicedata_changed(59, 3);
  ds->on_channelvalue_changed(60, 4, 125);

  ASSERT_TRUE(popMessage("SCOPE_FULL", 0, 0, 0, 0));

  ASSERT_EQ(ds->openCount(), 1);
  ASSERT_EQ(ds->closeCount(), 0);

  ASSERT_TRUE(popMessage("SCOPE_FULL", 0, 0, 0, 1));

  ASSERT_EQ(ds->openCount(), 1);
  ASSERT_EQ(ds->closeCount(), 1);

  ASSERT_FALSE(dataExists());
}

TEST_F(MqttDataSourceTest, onUserDataChanged) {
  ASSERT_EQ(ds->openCount(), 0);
  ASSERT_EQ(ds->closeCount(), 0);

  ds->on_channelvalue_changed(55, 1, 123);
  ds->on_devicedata_changed(55, 2);

  ds->on_userdata_changed(55);

  ds->on_devicedata_changed(55, 3);
  ds->on_channelvalue_changed(55, 4, 124);

  ds->on_userdata_changed(55);
  ds->on_userdata_changed(52);

  ASSERT_TRUE(popMessage("SCOPE_USER", 55, 0, 0, 0));

  ASSERT_EQ(ds->openCount(), 1);
  ASSERT_EQ(ds->closeCount(), 0);

  ASSERT_TRUE(popMessage("SCOPE_USER", 55, 0, 0, 1));

  ds->on_userdata_changed(52);

  ASSERT_EQ(ds->openCount(), 1);
  ASSERT_EQ(ds->closeCount(), 1);

  ASSERT_TRUE(popMessage("SCOPE_USER", 52, 0, 0, 0));

  ASSERT_EQ(ds->openCount(), 2);
  ASSERT_EQ(ds->closeCount(), 1);

  ASSERT_TRUE(popMessage("SCOPE_USER", 52, 0, 0, 1));

  ASSERT_EQ(ds->openCount(), 2);
  ASSERT_EQ(ds->closeCount(), 2);

  ASSERT_FALSE(dataExists());
}

TEST_F(MqttDataSourceTest, onDeviceDataChanged) {
  ASSERT_EQ(ds->openCount(), 0);
  ASSERT_EQ(ds->closeCount(), 0);

  ds->on_channelvalue_changed(55, 1, 123);
  ds->on_devicedata_changed(55, 1);
  ds->on_channelvalue_changed(55, 1, 124);

  ds->on_devicedata_changed(55, 1);
  ds->on_devicedata_changed(55, 2);

  // The identifiers should be unique in themselves
  ds->on_devicedata_changed(56, 1);
  //

  ASSERT_TRUE(popMessage("SCOPE_DEVICE", 55, 1, 0, 0));

  ASSERT_EQ(ds->openCount(), 1);
  ASSERT_EQ(ds->closeCount(), 0);

  ASSERT_TRUE(popMessage("SCOPE_DEVICE", 55, 1, 0, 1));

  ASSERT_EQ(ds->openCount(), 1);
  ASSERT_EQ(ds->closeCount(), 1);

  ASSERT_TRUE(popMessage("SCOPE_DEVICE", 55, 2, 0, 0));

  ASSERT_EQ(ds->openCount(), 2);
  ASSERT_EQ(ds->closeCount(), 1);

  ASSERT_TRUE(popMessage("SCOPE_DEVICE", 55, 2, 0, 1));

  ASSERT_EQ(ds->openCount(), 2);
  ASSERT_EQ(ds->closeCount(), 2);

  ASSERT_FALSE(dataExists());
}

TEST_F(MqttDataSourceTest, onChannelValueChanged) {
  ASSERT_EQ(ds->openCount(), 0);
  ASSERT_EQ(ds->closeCount(), 0);

  ds->on_channelvalue_changed(55, 15, 124);
  ds->on_channelvalue_changed(55, 15, 125);

  // The identifiers should be unique in themselves
  ds->on_channelvalue_changed(55, 20, 124);
  //

  ASSERT_TRUE(popMessage("CHANNEL_VALUE", 55, 15, 124, 0));

  ASSERT_EQ(ds->openCount(), 1);
  ASSERT_EQ(ds->closeCount(), 0);

  ASSERT_TRUE(popMessage("CHANNEL_VALUE", 55, 15, 124, 1));

  ASSERT_EQ(ds->openCount(), 1);
  ASSERT_EQ(ds->closeCount(), 1);

  ASSERT_TRUE(popMessage("CHANNEL_VALUE", 55, 15, 125, 0));

  ASSERT_EQ(ds->openCount(), 2);
  ASSERT_EQ(ds->closeCount(), 1);

  ASSERT_TRUE(popMessage("CHANNEL_VALUE", 55, 15, 125, 1));

  ASSERT_EQ(ds->openCount(), 2);
  ASSERT_EQ(ds->closeCount(), 2);

  ASSERT_FALSE(dataExists());
}

} /* namespace testing */
