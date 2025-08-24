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

#include "DeviceJsonCfgModePasswordTest.h"

#include "doubles/jsonconfig/device/DeviceJsonCfgModePasswordMock.h"

namespace testing {

DeviceJsonCfgModePasswordTest::DeviceJsonCfgModePasswordTest(void) {}

DeviceJsonCfgModePasswordTest::~DeviceJsonCfgModePasswordTest(void) {}

TEST_F(DeviceJsonCfgModePasswordTest, empty) {
  DeviceJsonCfgModePasswordMock ota;
  char *str = ota.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{}");
  free(str);

  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{}");
  free(str);
}

TEST_F(DeviceJsonCfgModePasswordTest, setAllVariantsAndVerifyReplacing) {
  DeviceJsonCfgModePasswordMock ota;
  EXPECT_CALL(ota, get_timestamp).WillRepeatedly(Return(1754227503));

  ota.set_waiting();
  char *str = ota.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{}");
  free(str);

  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"WAITING\"}}");
  free(str);

  ota.set_unauthorized();
  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"UNAUTHORIZED\"}}");
  free(str);

  ota.set_not_supported();
  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"NOT_SUPPORTED\"}}");
  free(str);

  ota.set_true();
  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"TRUE\"}}");
  free(str);

  ota.set_false();
  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"FALSE\"}}");
  free(str);
}

TEST_F(DeviceJsonCfgModePasswordTest, setCalCfgResult) {
  TDS_DeviceCalCfgResult result = {};
  result.Command = SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD;
  result.DataSize = 1;

  DeviceJsonCfgModePasswordMock json_result;
  EXPECT_CALL(json_result, get_timestamp).WillRepeatedly(Return(1754227503));

  EXPECT_FALSE(json_result.set_calcfg_result(&result));

  char *str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"FALSE\"}}");
  free(str);

  result.DataSize = 0;
  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"FALSE\"}}");
  free(str);

  result.Result = SUPLA_CALCFG_RESULT_TRUE;
  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"TRUE\"}}");
  free(str);

  result.Result = SUPLA_CALCFG_RESULT_UNAUTHORIZED;
  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"UNAUTHORIZED\"}}");
  free(str);

  result.Result = SUPLA_CALCFG_RESULT_NOT_SUPPORTED;
  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"setCfgModePassword\":{\"timestamp\":1754227503,\"status\":"
               "\"NOT_SUPPORTED\"}}");
  free(str);
}

TEST_F(DeviceJsonCfgModePasswordTest, merge) {
  DeviceJsonCfgModePasswordMock result1;

  result1.set_user_config("{\"x\":\"y\"}");

  result1.set_properties(
      "{\"a\":\"b\",\"setCfgModePassword\":{\"timestamp\":1754227503,"
      "\"status\":\"WAITING\"}}");

  DeviceJsonCfgModePasswordMock result2;
  result2.set_user_config("{\"c\":\"v\"}");
  result2.set_properties(
      "{\"setCfgModePassword\":{\"timestamp\":1754227600,\"status\":\"FALSE\"}"
      "}");
  result2.merge(&result1);

  char *str = result1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{\"x\":\"y\"}");

  free(str);

  str = result1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"setCfgModePassword\":{\"timestamp\":1754227600,"
               "\"status\":\"FALSE\"}}");

  free(str);
}

} /* namespace testing */
