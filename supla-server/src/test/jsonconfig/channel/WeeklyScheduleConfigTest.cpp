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
  TSD_ChannelConfig_WeeklySchedule sd_config1 = {};
  TSD_ChannelConfig_WeeklySchedule sd_config2 = {};

  sd_config1.Program[0].Mode = SUPLA_HVAC_MODE_COOL;
  sd_config1.Program[0].SetpointTemperatureMin = 10;
  sd_config1.Program[0].SetpointTemperatureMax = 20;

  sd_config1.Program[1].Mode = SUPLA_HVAC_MODE_DRY;
  sd_config1.Program[1].SetpointTemperatureMin = 30;
  sd_config1.Program[1].SetpointTemperatureMax = 40;

  sd_config1.Program[2].Mode = SUPLA_HVAC_MODE_FAN_ONLY;
  sd_config1.Program[2].SetpointTemperatureMin = 50;
  sd_config1.Program[2].SetpointTemperatureMax = 60;

  sd_config1.Program[3].Mode = SUPLA_HVAC_MODE_AUTO;
  sd_config1.Program[3].SetpointTemperatureMin = 70;
  sd_config1.Program[3].SetpointTemperatureMax = 80;

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
      "{\"hvac\":{\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":"
      "\"Cool\",\"setpointTemperatureMin\":10,\"setpointTemperatureMax\":20},"
      "\"2\":{\"mode\":\"Dry\",\"setpointTemperatureMin\":30,"
      "\"setpointTemperatureMax\":40},\"3\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureMin\":50,\"setpointTemperatureMax\":60},\"4\":{"
      "\"mode\":\"Auto\",\"setpointTemperatureMin\":70,"
      "\"setpointTemperatureMax\":80}},\"quarters\":[0,1,2,3,4,5,6,7,8,9,10,11,"
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
      "}}}");

  free(str);

  config.get_config(&sd_config2);

  EXPECT_EQ(memcmp(&sd_config1, &sd_config2,
                   sizeof(TSD_ChannelConfig_WeeklySchedule)),
            0);
}

TEST_F(WeeklyScheduleConfigTest, rendom) {
  TSD_ChannelConfig_WeeklySchedule sd_config1 = {};
  TSD_ChannelConfig_WeeklySchedule sd_config2 = {};

  TestHelper::randomize((char *)sd_config1.Quarters,
                        sizeof(sd_config1.Quarters));

  weekly_schedule_config config;
  config.set_config(&sd_config1);
  config.get_config(&sd_config2);

  EXPECT_EQ(memcmp(&sd_config1, &sd_config2,
                   sizeof(TSD_ChannelConfig_WeeklySchedule)),
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
      "{\"a\": 123, \"hvac\":{\"b\": 456, \"weeklySchedule\":{\"c\": 789}}}");

  config1.merge(&config2);

  str = config2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"a\":123,\"hvac\":{\"b\":456}}");
  free(str);

  config2.set_user_config(
      "{\"abc\": 123, "
      "\"hvac\":{\"x\": 987, "
      "\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":"
      "\"Cool\",\"setpointTemperatureMin\":10,\"setpointTemperatureMax\":20},"
      "\"2\":{\"mode\":\"Dry\",\"setpointTemperatureMin\":30,"
      "\"setpointTemperatureMax\":40},\"3\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureMin\":50,\"setpointTemperatureMax\":60},\"4\":{"
      "\"mode\":\"Auto\",\"setpointTemperatureMin\":70,"
      "\"setpointTemperatureMax\":80}},\"quarters\":[0,3,2,3,4,5,6,7,8,9,10,11,"
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
      "}}}");

  config1.set_user_config(
      "{\"a\": 123, \"hvac\":{\"b\": 456, "
      "\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureMin\":100,\"setpointTemperatureMax\":200},\"2\":{"
      "\"mode\":\"Dry\",\"setpointTemperatureMin\":300,"
      "\"setpointTemperatureMax\":400},\"3\":{\"mode\":\"FanOnly\","
      "\"setpointTemperatureMin\":500,\"setpointTemperatureMax\":600},\"4\":{"
      "\"mode\":\"Auto\",\"setpointTemperatureMin\":700,"
      "\"setpointTemperatureMax\":800}},\"quarters\":[0,1,2,3,4,5,6,7,8,9,10,"
      "11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,"
      "9]}}}");

  config1.merge(&config2);

  str = config2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"abc\":123,\"hvac\":{\"x\":987,\"weeklySchedule\":{"
               "\"programSettings\":{\"1\":{\"mode\":\"FanOnly\","
               "\"setpointTemperatureMin\":100,\"setpointTemperatureMax\":200},"
               "\"2\":{\"mode\":\"Dry\",\"setpointTemperatureMin\":300,"
               "\"setpointTemperatureMax\":400},\"3\":{\"mode\":\"FanOnly\","
               "\"setpointTemperatureMin\":500,\"setpointTemperatureMax\":600},"
               "\"4\":{\"mode\":\"Auto\",\"setpointTemperatureMin\":700,"
               "\"setpointTemperatureMax\":800}},\"quarters\":[0,1,2,3,4,5,6,7,"
               "8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,"
               "1,2,3,4,5,6,7,8,9]}}}");
  free(str);
}

} /* namespace testing */
