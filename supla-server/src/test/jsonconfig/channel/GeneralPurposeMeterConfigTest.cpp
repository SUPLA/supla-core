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

#include "GeneralPurposeMeterConfigTest.h"

#include "TestHelper.h"
#include "json/cJSON.h"
#include "jsonconfig/channel/general_purpose_meter_config.h"
#include "log.h"
#include "proto.h"

namespace testing {

GeneralPurposeMeterConfigTest::GeneralPurposeMeterConfigTest(void) {}

GeneralPurposeMeterConfigTest::~GeneralPurposeMeterConfigTest(void) {}

TEST_F(GeneralPurposeMeterConfigTest, setAndGetConfig) {
  TChannelConfig_GeneralPurposeMeter raw1 = {};
  TChannelConfig_GeneralPurposeMeter raw2 = {};
  raw1.ValueDivider = 12;
  raw1.ValueMultiplier = 34;
  raw1.ValueAdded = 56;
  raw1.ValuePrecision = 4;
  snprintf(raw1.UnitBeforeValue, sizeof(raw1.UnitBeforeValue), "ABCD");
  snprintf(raw1.UnitAfterValue, sizeof(raw1.UnitAfterValue), "EFGH");
  raw1.CounterType = SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_ALWAYS_INCREMENT;
  raw1.NoSpaceBeforeValue = 1;
  raw1.NoSpaceAfterValue = 1;
  raw1.KeepHistory = 1;
  raw1.RefreshIntervalMs = 345;
  raw1.ChartType = SUPLA_GENERAL_PURPOSE_METER_CHART_TYPE_BAR;
  raw1.DefaultValueDivider = 78;
  raw1.DefaultValueMultiplier = 910;
  raw1.DefaultValueAdded = 1112;
  raw1.DefaultValuePrecision = 2;
  raw1.IncludeValueAddedInHistory = 1;
  raw1.FillMissingData = 1;

  snprintf(raw1.DefaultUnitBeforeValue, sizeof(raw1.DefaultUnitBeforeValue),
           "XCVB");
  snprintf(raw1.DefaultUnitAfterValue, sizeof(raw1.DefaultUnitAfterValue),
           "GHJK");

  general_purpose_meter_config config1;
  config1.set_config(&raw1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"valueDivider\":12,\"valueMultiplier\":34,\"valueAdded\":56,"
      "\"valuePrecision\":4,\"unitBeforeValue\":\"ABCD\",\"unitAfterValue\":"
      "\"EFGH\",\"noSpaceBeforeValue\":true,\"noSpaceAfterValue\":true,"
      "\"keepHistory\":true,\"refreshIntervalMs\":345,\"chartType\":\"BAR\","
      "\"includeValueAddedInHistory\":true,\"fillMissingData\":true,"
      "\"counterType\":\"ALWAYS_INCREMENT\"}");

  general_purpose_meter_config config2;
  config2.set_user_config(str);
  free(str);

  str = config1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"defaultValueDivider\":78,\"defaultValueMultiplier\":910,"
               "\"defaultValueAdded\":1112,\"defaultValuePrecision\":2,"
               "\"defaultUnitBeforeValue\":\"XCVB\",\"defaultUnitAfterValue\":"
               "\"GHJK\"}");

  config2.set_properties(str);
  free(str);

  config1.get_config(&raw2);

  EXPECT_EQ(memcmp(&raw1, &raw2, sizeof(TChannelConfig_GeneralPurposeMeter)),
            0);
}

TEST_F(GeneralPurposeMeterConfigTest, booleans) {
  TChannelConfig_GeneralPurposeMeter raw = {};

  general_purpose_meter_config config;
  config.set_config(&raw);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"valueDivider\":0,\"valueMultiplier\":0,\"valueAdded\":0,"
      "\"valuePrecision\":0,\"unitBeforeValue\":\"\",\"unitAfterValue\":\"\","
      "\"noSpaceBeforeValue\":false,\"noSpaceAfterValue\":false,"
      "\"keepHistory\":false,\"refreshIntervalMs\":0,\"chartType\":\"BAR\","
      "\"includeValueAddedInHistory\":false,\"fillMissingData\":false,"
      "\"counterType\":\"INCREMENT_AND_DECREMENT\"}");
  free(str);

  raw.KeepHistory = 1;
  config.set_config(&raw);

  str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"valueDivider\":0,\"valueMultiplier\":0,\"valueAdded\":0,"
      "\"valuePrecision\":0,\"unitBeforeValue\":\"\",\"unitAfterValue\":\"\","
      "\"noSpaceBeforeValue\":false,\"noSpaceAfterValue\":false,"
      "\"refreshIntervalMs\":0,\"chartType\":\"BAR\","
      "\"includeValueAddedInHistory\":false,\"fillMissingData\":false,"
      "\"counterType\":\"INCREMENT_AND_DECREMENT\",\"keepHistory\":true}");
  free(str);

  raw.KeepHistory = 0;
  raw.IncludeValueAddedInHistory = 1;
  config.set_config(&raw);

  str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"valueDivider\":0,\"valueMultiplier\":0,\"valueAdded\":0,"
      "\"valuePrecision\":0,\"unitBeforeValue\":\"\",\"unitAfterValue\":\"\","
      "\"noSpaceBeforeValue\":false,\"noSpaceAfterValue\":false,"
      "\"refreshIntervalMs\":0,\"chartType\":\"BAR\",\"fillMissingData\":false,"
      "\"counterType\":\"INCREMENT_AND_DECREMENT\",\"keepHistory\":false,"
      "\"includeValueAddedInHistory\":true}");
  free(str);

  raw.IncludeValueAddedInHistory = 0;
  raw.FillMissingData = 1;
  config.set_config(&raw);

  str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"valueDivider\":0,\"valueMultiplier\":0,\"valueAdded\":0,"
      "\"valuePrecision\":0,\"unitBeforeValue\":\"\",\"unitAfterValue\":\"\","
      "\"noSpaceBeforeValue\":false,\"noSpaceAfterValue\":false,"
      "\"refreshIntervalMs\":0,\"chartType\":\"BAR\",\"counterType\":"
      "\"INCREMENT_AND_DECREMENT\",\"keepHistory\":false,"
      "\"includeValueAddedInHistory\":false,\"fillMissingData\":true}");
  free(str);

  raw.FillMissingData = 0;
  raw.NoSpaceBeforeValue = 1;
  config.set_config(&raw);

  str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"valueDivider\":0,\"valueMultiplier\":0,\"valueAdded\":0,"
      "\"valuePrecision\":0,\"unitBeforeValue\":\"\",\"unitAfterValue\":\"\","
      "\"noSpaceAfterValue\":false,\"refreshIntervalMs\":0,\"chartType\":"
      "\"BAR\",\"counterType\":\"INCREMENT_AND_DECREMENT\",\"keepHistory\":"
      "false,\"includeValueAddedInHistory\":false,\"noSpaceBeforeValue\":true,"
      "\"fillMissingData\":false}");
  free(str);

  raw.NoSpaceBeforeValue = 0;
  raw.NoSpaceAfterValue = 1;
  config.set_config(&raw);

  str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"valueDivider\":0,\"valueMultiplier\":0,\"valueAdded\":0,"
      "\"valuePrecision\":0,\"unitBeforeValue\":\"\",\"unitAfterValue\":\"\","
      "\"refreshIntervalMs\":0,\"chartType\":\"BAR\",\"counterType\":"
      "\"INCREMENT_AND_DECREMENT\",\"keepHistory\":false,"
      "\"includeValueAddedInHistory\":false,\"fillMissingData\":false,"
      "\"noSpaceBeforeValue\":false,\"noSpaceAfterValue\":true}");
  free(str);
}

TEST_F(GeneralPurposeMeterConfigTest, merge) {
  general_purpose_meter_config config1;
  config1.set_user_config(
      "{\"a\":\"b\",\"valueDivider\":12,\"valueMultiplier\":34,\"valueAdded\":"
      "56,\"valuePrecision\":5,\"unitBeforeValue\":\"ABCD\",\"unitAfterValue\":"
      "\"EFGH\",\"keepHistory\":true,\"chartType\":\"BAR\","
      "\"includeValueAddedInHistory\":true,\"fillMissingData\":true}");
  config1.set_properties(
      "{\"x\":\"y\",\"defaultValueDivider\":78,\"defaultValueMultiplier\":910,"
      "\"defaultValueAdded\":1112,\"defaultValuePrecision\":9,"
      "\"defaultUnitBeforeValue\":\"XCVB\",\"defaultUnitAfterValue\":"
      "\"GHJK\"}");

  general_purpose_meter_config config2;
  config2.set_user_config(
      "{\"keepHistory\":false,\"chartType\":\"LINEAR\",\"unitBeforeValue\":"
      "\"ABCD\"}");
  config2.set_properties("{\"defaultValueMultiplier\":95}");

  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"unitBeforeValue\":\"ABCD\",\"keepHistory\":"
               "false,\"chartType\":\"LINEAR\"}");

  free(str);

  str = config1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{\"x\":\"y\",\"defaultValueMultiplier\":95}");

  free(str);
}

} /* namespace testing */
