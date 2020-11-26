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

#include "MqttChannelMessageProviderTest.h"

namespace testing {

MqttChannelMessageProviderTest::MqttChannelMessageProviderTest(void)
    : MqttMessageProviderTest() {}
MqttChannelMessageProviderTest::~MqttChannelMessageProviderTest(void) {}

void MqttChannelMessageProviderTest::SetUp() {
  provider = new supla_mqtt_channel_message_provider();
}

void MqttChannelMessageProviderTest::TearDown() { delete provider; }

void MqttChannelMessageProviderTest::fillChannelData(
    _mqtt_db_data_row_channel_t *row_channel) {
  memset(row_channel, 0, sizeof(_mqtt_db_data_row_channel_t));

  snprintf(row_channel->user_email, SUPLA_EMAIL_MAXSIZE, "user@supla.org");
  snprintf(row_channel->user_shortuniqueid, SHORT_UNIQUEID_MAXSIZE,
           "7720767494dd87196e1896c7cbab707c");
  row_channel->device_id = 555;
  row_channel->device_enabled = true;
  row_channel->channel_id = 754;
  row_channel->channel_type = SUPLA_CHANNELTYPE_RELAY;
  row_channel->channel_func = SUPLA_CHANNELFNC_POWERSWITCH;
  snprintf(row_channel->channel_location, SUPLA_LOCATION_CAPTION_MAXSIZE,
           "Second floor");
  snprintf(row_channel->channel_caption, SUPLA_CHANNEL_CAPTION_MAXSIZE,
           "Socket");
  row_channel->channel_hidden = false;
}

TEST_F(MqttChannelMessageProviderTest, fetchAll) {
  _mqtt_db_data_row_channel_t row_channel;
  fillChannelData(&row_channel);
  provider->set_data_row(&row_channel);

  char device_id[15];
  snprintf(device_id, sizeof(device_id), "%i", row_channel.device_id);
  ASSERT_TRUE(fetchAndCompare(provider, "%email%", "555", false,
                              "user@supla.org/channels/%i/device_id",
                              row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(provider, "%email%", "RELAY", false,
                              "user@supla.org/channels/%i/type",
                              row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(provider, "%email%", "POWERSWITCH", false,
                              "user@supla.org/channels/%i/function",
                              row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(provider, "%email%", "Second floor", false,
                              "user@supla.org/channels/%i/location",
                              row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(provider, "%email%", "Socket", false,
                              "user@supla.org/channels/%i/caption",
                              row_channel.channel_id));

  ASSERT_TRUE(fetchAndCompare(provider, "%email%", "false", false,
                              "user@supla.org/channels/%i/hidden",
                              row_channel.channel_id));

  ASSERT_FALSE(dataExists(provider));
}

} /* namespace testing */
