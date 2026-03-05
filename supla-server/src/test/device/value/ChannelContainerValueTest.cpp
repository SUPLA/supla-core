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

#include "ChannelContainerValueTest.h"

#include "device/value/channel_container_value.h"

namespace testing {

TEST_F(ChannelContainerValueTest, getLevel) {
  TContainerChannel_Value raw = {};
  unsigned char level = 0;

  {
    supla_channel_container_value value(&raw);
    EXPECT_FALSE(value.get_level(&level));
    EXPECT_TRUE(value.is_invalid());
  }

  raw.level = 102;

  {
    supla_channel_container_value value(&raw);
    EXPECT_FALSE(value.get_level(&level));
    EXPECT_TRUE(value.is_invalid());
  }

  raw.level = 101;

  {
    supla_channel_container_value value(&raw);
    EXPECT_FALSE(value.get_level(nullptr));
    EXPECT_TRUE(value.get_level(&level));
    EXPECT_EQ(level, 100);
    EXPECT_FALSE(value.is_invalid());
  }

  raw.level = 1;

  {
    supla_channel_container_value value(&raw);
    EXPECT_TRUE(value.get_level(&level));
    EXPECT_EQ(level, 0);
    EXPECT_FALSE(value.is_invalid());
  }
}

TEST_F(ChannelContainerValueTest, warning) {
  TContainerChannel_Value raw = {};
  raw.flags = CONTAINER_FLAG_ALARM_LEVEL;

  {
    supla_channel_container_value value(&raw);
    EXPECT_FALSE(value.is_warning_flag_set());
  }

  raw.flags |= CONTAINER_FLAG_WARNING_LEVEL;

  {
    supla_channel_container_value value(&raw);
    EXPECT_TRUE(value.is_warning_flag_set());
  }
}

TEST_F(ChannelContainerValueTest, alarm) {
  TContainerChannel_Value raw = {};
  raw.flags = CONTAINER_FLAG_WARNING_LEVEL;

  {
    supla_channel_container_value value(&raw);
    EXPECT_FALSE(value.is_alarm_flag_set());
  }

  raw.flags |= CONTAINER_FLAG_ALARM_LEVEL;

  {
    supla_channel_container_value value(&raw);
    EXPECT_TRUE(value.is_alarm_flag_set());
  }
}

TEST_F(ChannelContainerValueTest, invalidSensorState) {
  TContainerChannel_Value raw = {};
  raw.flags = CONTAINER_FLAG_WARNING_LEVEL;

  {
    supla_channel_container_value value(&raw);
    EXPECT_FALSE(value.is_invalid_sensor_state_flag_set());
  }

  raw.flags |= CONTAINER_FLAG_INVALID_SENSOR_STATE;

  {
    supla_channel_container_value value(&raw);
    EXPECT_TRUE(value.is_invalid_sensor_state_flag_set());
  }
}

TEST_F(ChannelContainerValueTest, soundAlarm) {
  TContainerChannel_Value raw = {};
  raw.flags = CONTAINER_FLAG_WARNING_LEVEL;

  {
    supla_channel_container_value value(&raw);
    EXPECT_FALSE(value.is_sound_alarm_on());
  }

  raw.flags |= CONTAINER_FLAG_SOUND_ALARM_ON;

  {
    supla_channel_container_value value(&raw);
    EXPECT_TRUE(value.is_sound_alarm_on());
  }
}

TEST_F(ChannelContainerValueTest, templateData) {
  TContainerChannel_Value raw = {};
  raw.level = 10;
  supla_channel_container_value value(&raw);

  auto m = value.get_template_data();
  EXPECT_EQ(m.size(), 7);
  EXPECT_FALSE(m["invalid_value"].get<bool>());
  EXPECT_FALSE(m["alarm"].get<bool>());
  EXPECT_FALSE(m["sound_alarm_on"].get<bool>());
  EXPECT_FALSE(m["warning"].get<bool>());
  EXPECT_FALSE(m["invalid_sensor_state"].get<bool>());
  EXPECT_EQ(m["level"].get<int>(), 9);
  EXPECT_EQ(m["value"].get<int>(), 9);

  raw.level = 0;
  raw.flags =
      CONTAINER_FLAG_SOUND_ALARM_ON | CONTAINER_FLAG_INVALID_SENSOR_STATE;
  value = supla_channel_container_value(&raw);
  m = value.get_template_data();

  EXPECT_EQ(m.size(), 5);
  EXPECT_TRUE(m["invalid_value"].get<bool>());
  EXPECT_FALSE(m["alarm"].get<bool>());
  EXPECT_TRUE(m["sound_alarm_on"].get<bool>());
  EXPECT_FALSE(m["warning"].get<bool>());
  EXPECT_TRUE(m["invalid_sensor_state"].get<bool>());

  raw.flags = CONTAINER_FLAG_ALARM_LEVEL | CONTAINER_FLAG_WARNING_LEVEL;
  value = supla_channel_container_value(&raw);
  m = value.get_template_data();

  EXPECT_EQ(m.size(), 5);
  EXPECT_TRUE(m["invalid_value"].get<bool>());
  EXPECT_TRUE(m["alarm"].get<bool>());
  EXPECT_FALSE(m["sound_alarm_on"].get<bool>());
  EXPECT_TRUE(m["warning"].get<bool>());
  EXPECT_FALSE(m["invalid_sensor_state"].get<bool>());
}

}  // namespace testing
