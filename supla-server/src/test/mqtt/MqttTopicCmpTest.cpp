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

#include <list>
#include <string>

#include "MqttTopicCmpTest.h"
#include "mqtt_channelandstate_message_provider.h"

namespace testing {

MqttTopicCmpTest::MqttTopicCmpTest() {}
MqttTopicCmpTest::~MqttTopicCmpTest() {}

TEST_F(MqttTopicCmpTest, topicsRemoved) {
  supla_mqtt_channelandstate_message_provider *mp_before =
      new supla_mqtt_channelandstate_message_provider();
  supla_mqtt_channelandstate_message_provider *mp_after =
      new supla_mqtt_channelandstate_message_provider();

  _mqtt_db_data_row_channel_t channel_before;
  memset(&channel_before, 0, sizeof(_mqtt_db_data_row_channel_t));

  snprintf(channel_before.user_email, SUPLA_EMAIL_MAXSIZE, "user@supla.org");
  snprintf(channel_before.user_shortuniqueid, SHORT_UNIQUEID_MAXSIZE,
           "7720767494dd87196e1896c7cbab707c");
  channel_before.device_id = 555;
  channel_before.device_enabled = true;
  channel_before.channel_id = 754;
  channel_before.channel_type = SUPLA_CHANNELTYPE_BRIDGE;
  channel_before.channel_func = SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
  snprintf(channel_before.channel_location, SUPLA_LOCATION_CAPTION_MAXSIZE,
           "Second floor");
  snprintf(channel_before.channel_caption, SUPLA_CHANNEL_CAPTION_MAXSIZE,
           "Socket");
  channel_before.channel_hidden = false;

  _mqtt_db_data_row_channel_t channel_after;
  memcpy(&channel_after, &channel_before, sizeof(_mqtt_db_data_row_channel_t));
  channel_after.channel_func = SUPLA_CHANNELFNC_POWERSWITCH;

  mp_before->set_data_row(&channel_before);
  mp_after->set_data_row(&channel_after);

  std::list<std::string> removed =
      supla_mqtt_topic_cmp::topics_removed(NULL, mp_before, mp_after);

  delete mp_before;
  delete mp_after;

  ASSERT_EQ(removed.size(), (long unsigned int)3);
  std::list<std::string>::iterator rit = removed.begin();

  ASSERT_EQ(rit->compare("channels/754/state/hi"), 0);
  rit++;
  ASSERT_EQ(rit->compare("channels/754/state/is_calibrating"), 0);
  rit++;
  ASSERT_EQ(rit->compare("channels/754/state/shut"), 0);
}

} /* namespace testing */
