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

#include "MqttUserMessageProviderTest.h"

namespace testing {

MqttUserMessageProviderTest::MqttUserMessageProviderTest(void)
    : MqttMessageProviderTest() {}
MqttUserMessageProviderTest::~MqttUserMessageProviderTest(void) {}

void MqttUserMessageProviderTest::SetUp() {
  provider = new supla_mqtt_user_message_provider();
}

void MqttUserMessageProviderTest::TearDown() { delete provider; }

void MqttUserMessageProviderTest::fillUserData(
    _mqtt_db_data_row_user_t *row_user) {
  memset(row_user, 0, sizeof(_mqtt_db_data_row_user_t));
  row_user->user_id = 123;

  snprintf(row_user->user_email, SUPLA_EMAIL_MAXSIZE, "user@supla.org");
  snprintf(row_user->user_timezone, SUPLA_TIMEZONE_MAXSIZE, "Europe/Warsaw");
  snprintf(row_user->user_shortuniqueid, SHORT_UNIQUEID_MAXSIZE,
           "7720767494dd87196e1896c7cbab707c");
}

TEST_F(MqttUserMessageProviderTest, fetchEmpty) {
  char *topic_name = NULL;
  void *message = NULL;
  size_t message_size = 0;

  EXPECT_FALSE(provider->fetch("prefix", &topic_name, &message, &message_size));
  EXPECT_TRUE(topic_name == NULL);
  EXPECT_TRUE(message == NULL);
  EXPECT_EQ(message_size, (size_t)0);

  if (topic_name) {
    free(topic_name);
  }

  if (message) {
    free(message);
  }
}

TEST_F(MqttUserMessageProviderTest, fetchAllWithoutPrefix) {
  _mqtt_db_data_row_user_t row_user;
  fillUserData(&row_user);

  provider->set_data_row(&row_user);

  ASSERT_TRUE(
      fetchAndCompare(provider, "/account/timezone", "Europe/Warsaw", false));

  ASSERT_TRUE(fetchAndCompare(provider, "/account/shortuniqueid",
                              "7720767494dd87196e1896c7cbab707c", false));

  ASSERT_FALSE(dataExists(provider));
}

} /* namespace testing */
