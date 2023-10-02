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

#include "WeeklyScheduleConfigTest.h"

#include "TestHelper.h"
#include "json/cJSON.h"
#include "jsonconfig/channel/weekly_schedule_config.h"
#include "proto.h"

namespace testing {

WeeklyScheduleConfigTest::WeeklyScheduleConfigTest(void) {}

WeeklyScheduleConfigTest::~WeeklyScheduleConfigTest(void) {}

TEST_F(WeeklyScheduleConfigTest, setAndGetConfig) {
  TChannelConfig_WeeklySchedule sd_config1 = {};
  TChannelConfig_WeeklySchedule sd_config2 = {};

  sd_config1.Program[0].Mode = SUPLA_HVAC_MODE_COOL;
  sd_config1.Program[0].SetpointTemperatureHeat = 10;
  sd_config1.Program[0].SetpointTemperatureCool = 20;

  sd_config1.Program[1].Mode = SUPLA_HVAC_MODE_DRY;
  sd_config1.Program[1].SetpointTemperatureHeat = 30;
  sd_config1.Program[1].SetpointTemperatureCool = 40;

  sd_config1.Program[2].Mode = SUPLA_HVAC_MODE_FAN_ONLY;
  sd_config1.Program[2].SetpointTemperatureHeat = 50;
  sd_config1.Program[2].SetpointTemperatureCool = 60;

  sd_config1.Program[3].Mode = SUPLA_HVAC_MODE_AUTO;
  sd_config1.Program[3].SetpointTemperatureHeat = 70;
  sd_config1.Program[3].SetpointTemperatureCool = 80;

  unsigned short b = 0;
  for (unsigned short a = 0; a < sizeof(sd_config1.Quarters); a++) {
    unsigned char n = b % 16;
    b++;
    sd_config1.Quarters[a] = n;

    n = b % 16;
    b++;
    n <<= 4;
    sd_config1.Quarters[a] |= n;
  }

  weekly_schedule_config config;
  config.set_config(&sd_config1);

  char *str = config.get_user_config();
  ASSERT_TRUE(str != nullptr);

  EXPECT_STREQ(
      str,
      "{\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":"
      "\"COOL\",\"setpointTemperatureHeat\":10,\"setpointTemperatureCool\":20},"
      "\"2\":{\"mode\":\"DRY\",\"setpointTemperatureHeat\":30,"
      "\"setpointTemperatureCool\":40},\"3\":{\"mode\":\"FAN_ONLY\","
      "\"setpointTemperatureHeat\":50,\"setpointTemperatureCool\":60},\"4\":{"
      "\"mode\":\"AUTO\",\"setpointTemperatureHeat\":70,"
      "\"setpointTemperatureCool\":80}},\"quarters\":[0,1,2,3,4,5,6,7,8,9,10,"
      "11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,"
      "9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,"
      "7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,"
      "5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,"
      "3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,"
      "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,"
      "15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,"
      "13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,"
      "11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,"
      "9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,"
      "7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,"
      "5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,"
      "3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,"
      "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,"
      "15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,"
      "13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,"
      "11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,"
      "9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,"
      "7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,"
      "5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,"
      "3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,"
      "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,"
      "15]}}");

  free(str);

  config.get_config(&sd_config2);

  EXPECT_EQ(
      memcmp(&sd_config1, &sd_config2, sizeof(TChannelConfig_WeeklySchedule)),
      0);
}

TEST_F(WeeklyScheduleConfigTest, getConfigResult) {
  weekly_schedule_config config;
  TChannelConfig_WeeklySchedule sd_config = {};
  EXPECT_FALSE(config.get_config(&sd_config));

  config.set_user_config("{}");

  EXPECT_FALSE(config.get_config(&sd_config));

  config.set_user_config("{\"weeklySchedule\":{}}");

  EXPECT_FALSE(config.get_config(&sd_config));

  config.set_user_config("{\"weeklySchedule\":{\"programSettings\":{}}}");

  EXPECT_FALSE(config.get_config(&sd_config));

  config.set_user_config(
      "{\"weeklySchedule\":{\"programSettings\":{},\"quarters\":[]}}");

  EXPECT_FALSE(config.get_config(&sd_config));

  config.set_user_config(
      "{\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":\"COOL\"}},"
      "\"quarters\":[]}}");

  EXPECT_TRUE(config.get_config(&sd_config));

  config.set_user_config(
      "{\"weeklySchedule\":{\"programSettings\":{},\"quarters\":[0]}}");

  EXPECT_TRUE(config.get_config(&sd_config));
}

TEST_F(WeeklyScheduleConfigTest, rendom) {
  TChannelConfig_WeeklySchedule sd_config1 = {};
  TChannelConfig_WeeklySchedule sd_config2 = {};

  TestHelper::randomize((char *)sd_config1.Quarters,
                        sizeof(sd_config1.Quarters));

  weekly_schedule_config config;
  config.set_config(&sd_config1);
  config.get_config(&sd_config2);

  EXPECT_EQ(
      memcmp(&sd_config1, &sd_config2, sizeof(TChannelConfig_WeeklySchedule)),
      0);
}

TEST_F(WeeklyScheduleConfigTest, merge) {
  weekly_schedule_config config1;
  weekly_schedule_config config2;

  config1.merge(&config2);

  char *str = config2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{}");
  free(str);

  config2.set_user_config(
      "{\"a\": 123, \"b\": 456, \"weeklySchedule\":{\"c\": 789}}");

  config1.merge(&config2);

  str = config2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"a\":123,\"b\":456}");
  free(str);

  config2.set_user_config(
      "{\"abc\": 123, \"x\": 987, "
      "\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":"
      "\"COOL\",\"setpointTemperatureHeat\":10,\"setpointTemperatureCool\":20},"
      "\"2\":{\"mode\":\"Dry\",\"setpointTemperatureHeat\":30,"
      "\"setpointTemperatureCool\":40},\"3\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureHeat\":50,\"setpointTemperatureCool\":60},\"4\":{"
      "\"mode\":\"Auto\",\"setpointTemperatureHeat\":70,"
      "\"setpointTemperatureCool\":80}},\"quarters\":[0,3,2,3,4,5,6,7,8,9,10,"
      "11,"
      "12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,"
      "10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,"
      "8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,"
      "6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,"
      "4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,"
      "2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,"
      "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,"
      "14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,"
      "12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,"
      "10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,"
      "8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,"
      "6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,"
      "4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,"
      "2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,"
      "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,"
      "14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,"
      "12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,"
      "10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,"
      "8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,"
      "6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,"
      "4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,"
      "2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]"
      "}}");

  config1.set_user_config(
      "{\"a\": 123, \"b\": 456, "
      "\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureHeat\":100,\"setpointTemperatureCool\":200},\"2\":{"
      "\"mode\":\"Dry\",\"setpointTemperatureHeat\":300,"
      "\"setpointTemperatureCool\":400},\"3\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureHeat\":500,\"setpointTemperatureCool\":600},\"4\":{"
      "\"mode\":\"Auto\",\"setpointTemperatureHeat\":700,"
      "\"setpointTemperatureCool\":800}},\"quarters\":[0,1,2,3,4,5,6,7,8,9,10,"
      "11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,"
      "9]}}");

  config1.merge(&config2);

  str = config2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str,
      "{\"abc\":123,\"x\":987,\"weeklySchedule\":{"
      "\"programSettings\":{\"1\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureHeat\":100,\"setpointTemperatureCool\":200},"
      "\"2\":{\"mode\":\"Dry\",\"setpointTemperatureHeat\":300,"
      "\"setpointTemperatureCool\":400},\"3\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureHeat\":500,\"setpointTemperatureCool\":600},"
      "\"4\":{\"mode\":\"Auto\",\"setpointTemperatureHeat\":700,"
      "\"setpointTemperatureCool\":800}},\"quarters\":[0,1,2,3,4,5,6,7,"
      "8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,"
      "1,2,3,4,5,6,7,8,9]}}");
  free(str);
}

} /* namespace testing */
