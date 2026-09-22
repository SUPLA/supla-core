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
#include "cJSON.h"
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

  sd_config1.Program[3].Mode = SUPLA_HVAC_MODE_HEAT_COOL;
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
  config.set_config(&sd_config1, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);

  char *str = config.get_user_config();
  ASSERT_TRUE(str != nullptr);

  EXPECT_STREQ(
      str,
      "{\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":"
      "\"COOL\",\"setpointTemperatureHeat\":10,\"setpointTemperatureCool\":20},"
      "\"2\":{\"mode\":\"DRY\",\"setpointTemperatureHeat\":30,"
      "\"setpointTemperatureCool\":40},\"3\":{\"mode\":\"FAN_ONLY\","
      "\"setpointTemperatureHeat\":50,\"setpointTemperatureCool\":60},\"4\":{"
      "\"mode\":\"HEAT_COOL\",\"setpointTemperatureHeat\":70,"
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

  config.get_config(&sd_config2, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);

  EXPECT_EQ(
      memcmp(&sd_config1, &sd_config2, sizeof(TChannelConfig_WeeklySchedule)),
      0);
}

TEST_F(WeeklyScheduleConfigTest, getConfigResult) {
  weekly_schedule_config config;
  TChannelConfig_WeeklySchedule sd_config = {};
  EXPECT_FALSE(
      config.get_config(&sd_config, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  config.set_user_config("{}");

  EXPECT_FALSE(
      config.get_config(&sd_config, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  config.set_user_config("{\"weeklySchedule\":{}}");

  EXPECT_FALSE(
      config.get_config(&sd_config, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  config.set_user_config("{\"weeklySchedule\":{\"programSettings\":{}}}");

  EXPECT_FALSE(
      config.get_config(&sd_config, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  config.set_user_config(
      "{\"weeklySchedule\":{\"programSettings\":{},\"quarters\":[]}}");

  EXPECT_FALSE(
      config.get_config(&sd_config, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  config.set_user_config(
      "{\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":\"COOL\"}},"
      "\"quarters\":[]}}");

  EXPECT_TRUE(
      config.get_config(&sd_config, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));

  config.set_user_config(
      "{\"weeklySchedule\":{\"programSettings\":{},\"quarters\":[0]}}");

  EXPECT_TRUE(
      config.get_config(&sd_config, SUPLA_CHANNELFNC_HVAC_THERMOSTAT));
}

TEST_F(WeeklyScheduleConfigTest, rendom) {
  TChannelConfig_WeeklySchedule sd_config1 = {};
  TChannelConfig_WeeklySchedule sd_config2 = {};

  TestHelper::randomize((char *)sd_config1.Quarters,
                        sizeof(sd_config1.Quarters));

  weekly_schedule_config config;
  config.set_config(&sd_config1, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
  config.get_config(&sd_config2, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);

  EXPECT_EQ(
      memcmp(&sd_config1, &sd_config2, sizeof(TChannelConfig_WeeklySchedule)),
      0);
}

TEST_F(WeeklyScheduleConfigTest, relayProgramsAreSerializedByFunction) {
  const _supla_int_t relay_functions[] = {
      SUPLA_CHANNELFNC_LIGHTSWITCH,
      SUPLA_CHANNELFNC_POWERSWITCH,
      SUPLA_CHANNELFNC_STAIRCASETIMER,
      SUPLA_CHANNELFNC_CONTROLLINGTHEGATE,
      SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK,
      SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR,
      SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK};

  for (auto func : relay_functions) {
    TChannelConfig_WeeklySchedule sd_config1 = {};
    TChannelConfig_WeeklySchedule sd_config2 = {};

    sd_config1.Program[0].Mode = SUPLA_RELAY_MODE_START_ON;
    sd_config1.Program[0].RelayModeDurationS = 65535;
    sd_config1.Program[0].RelayOppositeModeDurationS = 123;
    sd_config1.Program[1].Mode = SUPLA_RELAY_MODE_START_OFF;
    sd_config1.Program[1].RelayModeDurationS = 321;
    sd_config1.Program[2].Mode = SUPLA_RELAY_MODE_FORCED_ON;
    sd_config1.Program[3].Mode = SUPLA_RELAY_MODE_AUTOMATIC;

    weekly_schedule_config config;
    config.set_config(&sd_config1, func);

    char *str = config.get_user_config();
    ASSERT_NE(str, nullptr);

    cJSON *root = cJSON_Parse(str);
    ASSERT_NE(root, nullptr);
    cJSON *weekly_schedule = cJSON_GetObjectItem(root, "weeklySchedule");
    ASSERT_NE(weekly_schedule, nullptr);
    cJSON *program_settings =
        cJSON_GetObjectItem(weekly_schedule, "programSettings");
    ASSERT_NE(program_settings, nullptr);
    cJSON *program = cJSON_GetObjectItem(program_settings, "1");
    ASSERT_NE(program, nullptr);

    EXPECT_STREQ(cJSON_GetStringValue(cJSON_GetObjectItem(program, "mode")),
                 "START_ON");
    EXPECT_EQ(cJSON_GetNumberValue(
                  cJSON_GetObjectItem(program, "relayModeDurationS")),
              65535);
    EXPECT_EQ(cJSON_GetNumberValue(
                  cJSON_GetObjectItem(program,
                                      "relayOppositeModeDurationS")),
              123);
    EXPECT_EQ(cJSON_GetObjectItem(program, "setpointTemperatureHeat"),
              nullptr);
    EXPECT_EQ(cJSON_GetObjectItem(program, "setpointTemperatureCool"),
              nullptr);

    cJSON_Delete(root);
    free(str);

    ASSERT_TRUE(config.get_config(&sd_config2, func));
    EXPECT_EQ(
        memcmp(&sd_config1, &sd_config2,
               sizeof(TChannelConfig_WeeklySchedule)),
        0);
  }
}

TEST_F(WeeklyScheduleConfigTest, actionTriggerProgramsUseButtonMode) {
  TChannelConfig_WeeklySchedule sd_config1 = {};
  TChannelConfig_WeeklySchedule sd_config2 = {};

  sd_config1.Program[0].Mode = SUPLA_BUTTON_MODE_LOCKED;
  sd_config1.Program[1].Mode = SUPLA_BUTTON_MODE_NOT_SET;

  weekly_schedule_config config;
  config.set_config(&sd_config1, SUPLA_CHANNELFNC_ACTIONTRIGGER);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);

  cJSON *root = cJSON_Parse(str);
  ASSERT_NE(root, nullptr);
  cJSON *weekly_schedule = cJSON_GetObjectItem(root, "weeklySchedule");
  ASSERT_NE(weekly_schedule, nullptr);
  cJSON *program_settings =
      cJSON_GetObjectItem(weekly_schedule, "programSettings");
  ASSERT_NE(program_settings, nullptr);
  cJSON *program = cJSON_GetObjectItem(program_settings, "1");
  ASSERT_NE(program, nullptr);

  EXPECT_STREQ(cJSON_GetStringValue(cJSON_GetObjectItem(program, "mode")),
               "LOCKED");
  EXPECT_EQ(cJSON_GetObjectItem(program, "setpointTemperatureHeat"), nullptr);
  EXPECT_EQ(cJSON_GetObjectItem(program, "setpointTemperatureCool"), nullptr);
  EXPECT_EQ(cJSON_GetObjectItem(program, "relayModeDurationS"), nullptr);
  EXPECT_EQ(cJSON_GetObjectItem(program, "relayOppositeModeDurationS"),
            nullptr);

  cJSON_Delete(root);
  free(str);

  ASSERT_TRUE(
      config.get_config(&sd_config2, SUPLA_CHANNELFNC_ACTIONTRIGGER));
  EXPECT_EQ(
      memcmp(&sd_config1, &sd_config2,
             sizeof(TChannelConfig_WeeklySchedule)),
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
      "\"2\":{\"mode\":\"DRY\",\"setpointTemperatureHeat\":30,"
      "\"setpointTemperatureCool\":40},\"3\":{\"mode\":\"FAN_ONLY\","
      "\"setpointTemperatureHeat\":50,\"setpointTemperatureCool\":60},\"4\":{"
      "\"mode\":\"HEAT_COOL\",\"setpointTemperatureHeat\":70,"
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
      "\"weeklySchedule\":{\"programSettings\":{\"1\":{\"mode\":\"FAN_ONLY\","
      "\"setpointTemperatureHeat\":100,\"setpointTemperatureCool\":200},\"2\":{"
      "\"mode\":\"DRY\",\"setpointTemperatureHeat\":300,"
      "\"setpointTemperatureCool\":400},\"3\":{\"mode\":\"FAN_ONLY\","
      "\"setpointTemperatureHeat\":500,\"setpointTemperatureCool\":600},\"4\":{"
      "\"mode\":\"HEAT_COOL\",\"setpointTemperatureHeat\":700,"
      "\"setpointTemperatureCool\":800}},\"quarters\":[0,1,2,3,4,5,6,7,8,9,10,"
      "11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,"
      "9]}}");

  config1.merge(&config2);

  str = config2.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str,
      "{\"abc\":123,\"x\":987,\"weeklySchedule\":{"
      "\"programSettings\":{\"1\":{\"mode\":\"FAN_ONLY\","
      "\"setpointTemperatureHeat\":100,\"setpointTemperatureCool\":200},"
      "\"2\":{\"mode\":\"DRY\",\"setpointTemperatureHeat\":300,"
      "\"setpointTemperatureCool\":400},\"3\":{\"mode\":\"FAN_ONLY\","
      "\"setpointTemperatureHeat\":500,\"setpointTemperatureCool\":600},"
      "\"4\":{\"mode\":\"HEAT_COOL\",\"setpointTemperatureHeat\":700,"
      "\"setpointTemperatureCool\":800}},\"quarters\":[0,1,2,3,4,5,6,7,"
      "8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,"
      "1,2,3,4,5,6,7,8,9]}}");
  free(str);
}

} /* namespace testing */
