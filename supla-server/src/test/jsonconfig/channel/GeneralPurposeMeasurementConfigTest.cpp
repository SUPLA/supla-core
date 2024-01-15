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

#include "GeneralPurposeMeasurementConfigTest.h"

#include "TestHelper.h"
#include "json/cJSON.h"
#include "jsonconfig/channel/general_purpose_measurement_config.h"
#include "log.h"
#include "proto.h"

namespace testing {

GeneralPurposeMeasurementConfigTest::GeneralPurposeMeasurementConfigTest(void) {
}

GeneralPurposeMeasurementConfigTest::~GeneralPurposeMeasurementConfigTest(
    void) {}

TEST_F(GeneralPurposeMeasurementConfigTest, setAndGetConfig) {
  TChannelConfig_GeneralPurposeMeasurement raw1 = {};
  TChannelConfig_GeneralPurposeMeasurement raw2 = {};
  raw1.ValueDivider = 12;
  raw1.ValueMultiplier = 34;
  raw1.ValueAdded = 56;
  raw1.ValuePrecision = 5;
  snprintf(raw1.UnitBeforeValue, sizeof(raw1.UnitBeforeValue), "ABCD");
  snprintf(raw1.UnitAfterValue, sizeof(raw1.UnitAfterValue), "EFGH");
  raw1.NoSpaceAfterValue = 1;
  raw1.KeepHistory = 1;
  raw1.ChartType = SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_CANDLE;
  raw1.DefaultValueDivider = 78;
  raw1.DefaultValueMultiplier = 910;
  raw1.DefaultValueAdded = 1112;
  raw1.DefaultValuePrecision = 9;
  snprintf(raw1.DefaultUnitBeforeValue, sizeof(raw1.DefaultUnitBeforeValue),
           "XCVB");
  snprintf(raw1.DefaultUnitAfterValue, sizeof(raw1.DefaultUnitAfterValue),
           "GHJK");

  general_purpose_measurement_config config1;
  config1.set_config(&raw1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"valueDivider\":12,\"valueMultiplier\":34,\"valueAdded\":56,"
               "\"valuePrecision\":5,\"unitBeforeValue\":\"ABCD\","
               "\"unitAfterValue\":\"EFGH\",\"noSpaceAfterValue\":true,"
               "\"keepHistory\":true,\"chartType\":\"CANDLE\"}");

  general_purpose_measurement_config config2;
  config2.set_user_config(str);
  free(str);

  str = config1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"defaultValueDivider\":78,\"defaultValueMultiplier\":910,"
               "\"defaultValueAdded\":1112,\"defaultValuePrecision\":9,"
               "\"defaultUnitBeforeValue\":\"XCVB\",\"defaultUnitAfterValue\":"
               "\"GHJK\"}");

  config2.set_properties(str);
  free(str);

  config1.get_config(&raw2);

  EXPECT_EQ(
      memcmp(&raw1, &raw2, sizeof(TChannelConfig_GeneralPurposeMeasurement)),
      0);
}

TEST_F(GeneralPurposeMeasurementConfigTest, merge) {
  general_purpose_measurement_config config1;
  config1.set_user_config(
      "{\"a\":\"b\",\"valueDivider\":12,\"valueMultiplier\":34,\"valueAdded\":"
      "56,\"valuePrecision\":5,\"unitAfterValue\":\"EFGH\",\"keepHistory\":"
      "true,\"chartType\":\"CANDLE\"}");
  config1.set_properties(
      "{\"x\":\"y\",\"defaultValueDivider\":78,\"defaultValueMultiplier\":910,"
      "\"defaultValueAdded\":1112,\"defaultValuePrecision\":9,"
      "\"defaultUnitBeforeValue\":\"XCVB\",\"defaultUnitAfterValue\":"
      "\"GHJK\"}");

  general_purpose_measurement_config config2;
  config2.set_user_config(
      "{\"keepHistory\":false,\"chartType\":\"BAR\",\"unitBeforeValue\":"
      "\"ABCD\"}");
  config2.set_properties("{\"defaultValueMultiplier\":95}");

  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"keepHistory\":false,\"chartType\":\"BAR\","
               "\"unitBeforeValue\":\"ABCD\"}");

  free(str);

  str = config1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{\"x\":\"y\",\"defaultValueMultiplier\":95}");

  free(str);
}

} /* namespace testing */
