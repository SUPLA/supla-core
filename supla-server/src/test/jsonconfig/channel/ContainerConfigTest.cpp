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

#include "ContainerConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/container_config.h"

namespace testing {

ContainerConfigTest::ContainerConfigTest(void) {}

ContainerConfigTest::~ContainerConfigTest(void) {}

TEST_F(ContainerConfigTest, setAndGetConfig) {
  TChannelConfig_Container raw1 = {};
  raw1.WarningAboveLevel = 20;
  raw1.AlarmAboveLevel = 30;
  raw1.WarningBelowLevel = 50;
  raw1.AlarmBelowLevel = 40;

  for (size_t a = 0;
       a < sizeof(raw1.SensorInfo) / sizeof(TContainer_SensorInfo); a++) {
    raw1.SensorInfo[a].FillLevel = 11 + a;
    raw1.SensorInfo[a].ChannelNo = a;
    raw1.SensorInfo[a].IsSet = 1;
  }

  container_config config1;
  config1.set_config(&raw1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"warningAboveLevel\":19,\"alarmAboveLevel\":29,\"warningBelowLevel\":"
      "49,\"alarmBelowLevel\":39,\"muteAlarmSoundWithoutAdditionalAuth\":false,"
      "\"sensors\":{\"0\":{\"fillLevel\":10},\"1\":{\"fillLevel\":11},\"2\":{"
      "\"fillLevel\":12},\"3\":{\"fillLevel\":13},\"4\":{\"fillLevel\":14},"
      "\"5\":{\"fillLevel\":15},\"6\":{\"fillLevel\":16},\"7\":{\"fillLevel\":"
      "17},\"8\":{\"fillLevel\":18},\"9\":{\"fillLevel\":19}}}");

  container_config config2;
  config2.set_user_config(str);
  free(str);

  TChannelConfig_Container raw2 = {};
  EXPECT_TRUE(config2.get_config(&raw2));

  EXPECT_EQ(0, memcmp(&raw1, &raw2, sizeof(raw1)));
}


} /* namespace testing */
