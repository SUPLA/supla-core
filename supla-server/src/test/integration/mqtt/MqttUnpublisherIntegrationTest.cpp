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

#include "MqttUnpublisherIntegrationTest.h"
#include "log.h"
#include "mqtt_unpublisher.h"
#include "mqtt_unpublisher_datasource.h"

namespace testing {

MqttUnpublisherIntegrationTest::MqttUnpublisherIntegrationTest()
    : MqttClientIntegrationTest() {}

MqttUnpublisherIntegrationTest::~MqttUnpublisherIntegrationTest() {}

supla_mqtt_client *MqttUnpublisherIntegrationTest::clientInit(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource) {
  return new supla_mqtt_unpublisher(library_adapter, settings, datasource);
}

supla_mqtt_client_datasource *MqttUnpublisherIntegrationTest::dsInit(
    supla_mqtt_client_settings *settings) {
  return new supla_mqtt_unpublisher_datasource(settings);
}

void MqttUnpublisherIntegrationTest::SetUp() {
  initTestDatabase();
  runSqlScript("DataForMqttTests.sql");
  runSqlScript("DisableMqttForUser2645.sql");

  MqttClientIntegrationTest::SetUp();
}

TEST_F(MqttUnpublisherIntegrationTest, disableAlreadyDisabled) {
  waitForConnection();

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  getDS()->on_userdata_changed(2645);
  sleep(2);

  ASSERT_FALSE(getDS()->is_context_open());
  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);
}

TEST_F(MqttUnpublisherIntegrationTest, enableDisabledAndDisableAgain) {
  waitForConnection();

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  runSqlScript("EnableMqttForUser2645.sql");

  getDS()->on_userdata_changed(2645);
  sleep(2);

  ASSERT_FALSE(getDS()->is_context_open());
  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  runSqlScript("DisableMqttForUser2645.sql");

  getDS()->on_userdata_changed(2645);
  sleep(2);

  ASSERT_FALSE(getDS()->is_context_open());
  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 2);

  ASSERT_EQ(
      getLibAdapter()->subscribed_pop().compare("supla/2645test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->subscribed_pop().compare(
                "homeassistant/+/6527881802ebd8c4ff2b3955bc326704/+/config"),
            0);

  sleep(22);

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 2);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  ASSERT_EQ(
      getLibAdapter()->unsubscribed_pop().compare("supla/2645test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->unsubscribed_pop().compare(
                "homeassistant/+/6527881802ebd8c4ff2b3955bc326704/+/config"),
            0);

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  runSqlScript("EnableMqttForUser2645.sql");

  getDS()->on_userdata_changed(2645);
  sleep(2);

  ASSERT_FALSE(getDS()->is_context_open());
  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  runSqlScript("DisableMqttForUser2645.sql");

  getDS()->on_userdata_changed(2645);
  sleep(2);

  ASSERT_FALSE(getDS()->is_context_open());
  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 2);

  ASSERT_EQ(
      getLibAdapter()->subscribed_pop().compare("supla/2645test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->subscribed_pop().compare(
                "homeassistant/+/6527881802ebd8c4ff2b3955bc326704/+/config"),
            0);

  runSqlScript("EnableMqttForUser2645.sql");

  getDS()->on_userdata_changed(2645);
  sleep(2);

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 2);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  ASSERT_EQ(
      getLibAdapter()->unsubscribed_pop().compare("supla/2645test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->unsubscribed_pop().compare(
                "homeassistant/+/6527881802ebd8c4ff2b3955bc326704/+/config"),
            0);
}

TEST_F(MqttUnpublisherIntegrationTest, disableEnabled) {
  waitForConnection();

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  runSqlScript("DisableMqttForUser48.sql");

  getDS()->on_userdata_changed(48);
  sleep(2);

  ASSERT_FALSE(getDS()->is_context_open());
  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 2);

  ASSERT_EQ(
      getLibAdapter()->subscribed_pop().compare("supla/48test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->subscribed_pop().compare(
                "homeassistant/+/8ce92cb8c9f6db6b65703d2703691700/+/config"),
            0);

  sleep(22);

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 2);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  ASSERT_EQ(
      getLibAdapter()->unsubscribed_pop().compare("supla/48test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->unsubscribed_pop().compare(
                "homeassistant/+/8ce92cb8c9f6db6b65703d2703691700/+/config"),
            0);
}

TEST_F(MqttUnpublisherIntegrationTest, enableDuringDisabling) {
  waitForConnection();

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  runSqlScript("DisableMqttForUser48.sql");

  getDS()->on_userdata_changed(48);
  sleep(2);

  ASSERT_FALSE(getDS()->is_context_open());
  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 2);

  ASSERT_EQ(
      getLibAdapter()->subscribed_pop().compare("supla/48test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->subscribed_pop().compare(
                "homeassistant/+/8ce92cb8c9f6db6b65703d2703691700/+/config"),
            0);

  sleep(5);

  runSqlScript("EnableMqttForUser48.sql");

  getDS()->on_userdata_changed(48);
  sleep(2);

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 2);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  ASSERT_EQ(
      getLibAdapter()->unsubscribed_pop().compare("supla/48test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->unsubscribed_pop().compare(
                "homeassistant/+/8ce92cb8c9f6db6b65703d2703691700/+/config"),
            0);

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  runSqlScript("DisableMqttForUser48.sql");

  getDS()->on_userdata_changed(48);
  sleep(2);

  ASSERT_FALSE(getDS()->is_context_open());
  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 0);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 2);

  ASSERT_EQ(
      getLibAdapter()->subscribed_pop().compare("supla/48test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->subscribed_pop().compare(
                "homeassistant/+/8ce92cb8c9f6db6b65703d2703691700/+/config"),
            0);

  sleep(22);

  ASSERT_EQ(getLibAdapter()->unsubscribed_count(), 2);
  ASSERT_EQ(getLibAdapter()->published_count(), 0);
  ASSERT_EQ(getLibAdapter()->subscribed_count(), 0);

  ASSERT_EQ(
      getLibAdapter()->unsubscribed_pop().compare("supla/48test@supla.org/#"),

      0);

  ASSERT_EQ(getLibAdapter()->unsubscribed_pop().compare(
                "homeassistant/+/8ce92cb8c9f6db6b65703d2703691700/+/config"),
            0);
}
} /* namespace testing */
