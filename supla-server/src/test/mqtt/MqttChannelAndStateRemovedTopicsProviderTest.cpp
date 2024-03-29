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

#include "MqttChannelAndStateRemovedTopicsProviderTest.h"

#include "mqtt_db.h"

namespace testing {

MqttChannelAndStateRemovedTopicsProviderTest::
    MqttChannelAndStateRemovedTopicsProviderTest(void)
    : MqttMessageProviderTest() {
  removed_topics_provider = NULL;
}
MqttChannelAndStateRemovedTopicsProviderTest::
    ~MqttChannelAndStateRemovedTopicsProviderTest(void) {}

void MqttChannelAndStateRemovedTopicsProviderTest::SetUp() {
  removed_topics_provider =
      new supla_mqtt_channelandstate_removed_topics_provider();
}

void MqttChannelAndStateRemovedTopicsProviderTest::TearDown() {
  delete removed_topics_provider;
}

TEST_F(MqttChannelAndStateRemovedTopicsProviderTest, setFunctionToNone) {
  _mqtt_db_data_row_channel_t before = {};
  _mqtt_db_data_row_channel_t after = {};

  snprintf(before.user_suid, SHORT_UNIQUEID_MAXSIZE,
           "7720767494dd87196e1896c7cbab707c");
  before.device_id = 555;
  before.device_enabled = true;
  before.channel_id = 754;
  before.channel_type = SUPLA_CHANNELTYPE_RELAY;
  before.channel_func = SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
  snprintf(before.channel_location, SUPLA_LOCATION_CAPTION_MAXSIZE,
           "Second floor");
  snprintf(before.channel_caption, SUPLA_CHANNEL_CAPTION_MAXSIZE,
           "Roller Shutter 1");
  before.channel_hidden = false;

  after = before;
  after.channel_func = SUPLA_CHANNELFNC_NONE;

  removed_topics_provider->set_data(NULL, &before, &after);

  ASSERT_TRUE(fetchAndCompare(
      removed_topics_provider, NULL, NULL, false,
      "homeassistant/cover/7720767494dd87196e1896c7cbab707c/754/config"));

  ASSERT_TRUE(fetchAndCompare(removed_topics_provider, NULL, NULL, false,
                              "supla/7720767494dd87196e1896c7cbab707c/devices/"
                              "555/channels/754/state/hi"));

  ASSERT_TRUE(fetchAndCompare(removed_topics_provider, NULL, NULL, false,
                              "supla/7720767494dd87196e1896c7cbab707c/devices/"
                              "555/channels/754/state/is_calibrating"));

  ASSERT_TRUE(fetchAndCompare(removed_topics_provider, NULL, NULL, false,
                              "supla/7720767494dd87196e1896c7cbab707c/devices/"
                              "555/channels/754/state/shut"));

  ASSERT_FALSE(dataExists(removed_topics_provider));
}

} /* namespace testing */
