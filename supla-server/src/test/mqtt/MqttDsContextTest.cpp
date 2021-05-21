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

#include "MqttDsContextTest.h"
#include "mqtt_ds_context.h"

namespace testing {

MqttDsContextTest::MqttDsContextTest(void) {}

MqttDsContextTest::~MqttDsContextTest(void) {}

TEST_F(MqttDsContextTest, emptyConstructor) {
  supla_mqtt_ds_context context;
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_NONE);
  ASSERT_EQ(context.get_user_id(), 0);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);
}

TEST_F(MqttDsContextTest, constructorWithScope) {
  supla_mqtt_ds_context context(MQTTDS_SCOPE_FULL);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_FULL);
  ASSERT_EQ(context.get_user_id(), 0);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);
}

TEST_F(MqttDsContextTest, constructorWithUserID) {
  supla_mqtt_ds_context context(MQTTDS_SCOPE_USER, 123);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_USER);
  ASSERT_EQ(context.get_user_id(), 123);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);

  context = supla_mqtt_ds_context(MQTTDS_SCOPE_FULL, 123);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_FULL);
  ASSERT_EQ(context.get_user_id(), 0);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);
}

TEST_F(MqttDsContextTest, constructorWithDeviceID) {
  supla_mqtt_ds_context context(MQTTDS_SCOPE_DEVICE, 123, 33);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_DEVICE);
  ASSERT_EQ(context.get_user_id(), 123);
  ASSERT_EQ(context.get_device_id(), 33);
  ASSERT_EQ(context.get_channel_id(), 0);

  context = supla_mqtt_ds_context(MQTTDS_SCOPE_USER, 123, 33);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_USER);
  ASSERT_EQ(context.get_user_id(), 123);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);

  context = supla_mqtt_ds_context(MQTTDS_SCOPE_FULL, 123, 33);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_FULL);
  ASSERT_EQ(context.get_user_id(), 0);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);
}

TEST_F(MqttDsContextTest, constructorWithChannelID) {
  supla_mqtt_ds_context context(MQTTDS_SCOPE_CHANNEL_STATE, 123, 33, 5);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_CHANNEL_STATE);
  ASSERT_EQ(context.get_user_id(), 123);
  ASSERT_EQ(context.get_device_id(), 33);
  ASSERT_EQ(context.get_channel_id(), 5);

  context = supla_mqtt_ds_context(MQTTDS_SCOPE_DEVICE, 123, 33, 5);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_DEVICE);
  ASSERT_EQ(context.get_user_id(), 123);
  ASSERT_EQ(context.get_device_id(), 33);
  ASSERT_EQ(context.get_channel_id(), 0);

  context = supla_mqtt_ds_context(MQTTDS_SCOPE_USER, 123, 33, 5);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_USER);
  ASSERT_EQ(context.get_user_id(), 123);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);

  context = supla_mqtt_ds_context(MQTTDS_SCOPE_FULL, 123, 33, 5);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_FULL);
  ASSERT_EQ(context.get_user_id(), 0);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);
}

TEST_F(MqttDsContextTest, userIdSetter) {
  supla_mqtt_ds_context context(MQTTDS_SCOPE_FULL);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_FULL);
  ASSERT_EQ(context.get_user_id(), 0);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);

  context.set_user_id(10);
  ASSERT_EQ(context.get_user_id(), 10);
  context.set_user_id(20);
  ASSERT_EQ(context.get_user_id(), 10);
  context.set_user_id(0);
  ASSERT_EQ(context.get_user_id(), 0);
  context.set_user_id(20);
  ASSERT_EQ(context.get_user_id(), 20);

  context = supla_mqtt_ds_context(MQTTDS_SCOPE_USER, 123, 33);
  ASSERT_EQ(context.get_scope(), MQTTDS_SCOPE_USER);
  ASSERT_EQ(context.get_user_id(), 123);
  ASSERT_EQ(context.get_device_id(), 0);
  ASSERT_EQ(context.get_channel_id(), 0);

  context.set_user_id(0);
  ASSERT_EQ(context.get_user_id(), 123);

  context.set_user_id(10);
  ASSERT_EQ(context.get_user_id(), 123);
}

} /* namespace testing */
