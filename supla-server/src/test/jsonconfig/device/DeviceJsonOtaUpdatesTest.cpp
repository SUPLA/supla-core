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

#include "DeviceJsonOtaUpdatesTest.h"

#include "doubles/jsonconfig/device/DeviceJsonOtaUpdatesMock.h"

namespace testing {

DeviceJsonOtaUpdatesTest::DeviceJsonOtaUpdatesTest(void) {}

DeviceJsonOtaUpdatesTest::~DeviceJsonOtaUpdatesTest(void) {}

TEST_F(DeviceJsonOtaUpdatesTest, empty) {
  DeviceJsonOtaUpdatesMock ota;
  char *str = ota.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{}");
  free(str);

  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{}");
  free(str);
}

TEST_F(DeviceJsonOtaUpdatesTest, setAllVariantsAndVerifyReplacing) {
  DeviceJsonOtaUpdatesMock ota;
  EXPECT_CALL(ota, get_timestamp).WillRepeatedly(Return(1754227503));

  ota.set_error();
  char *str = ota.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{}");
  free(str);

  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str, "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"error\"}}");
  free(str);

  ota.set_checking();
  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"checking\"}}");
  free(str);

  ota.set_not_available();
  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"notAvailable\"}}");
  free(str);

  ota.set_available("25.08", "https://moon.update.com");
  str = ota.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"available\","
      "\"version\":\"25.08\",\"url\":\"https://moon.update.com\"}}");
  free(str);
}

TEST_F(DeviceJsonOtaUpdatesTest, setCalCfgResultWithFailure) {
  TDS_DeviceCalCfgResult result = {};
  result.Command = SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE;

  DeviceJsonOtaUpdatesMock json_result;
  EXPECT_CALL(json_result, get_timestamp).WillRepeatedly(Return(1754227503));

  EXPECT_FALSE(json_result.set_calcfg_result(&result));

  char *str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str, "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"error\"}}");
  free(str);

  result.DataSize = sizeof(TCalCfg_FirmwareCheckResult);
  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"notAvailable\"}}");
  free(str);

  ((TCalCfg_FirmwareCheckResult *)result.Data)->Result =
      SUPLA_FIRMWARE_CHECK_RESULT_UPDATE_NOT_AVAILABLE;
  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"notAvailable\"}}");
  free(str);

  ((TCalCfg_FirmwareCheckResult *)result.Data)->Result =
      SUPLA_FIRMWARE_CHECK_RESULT_ERROR;
  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str, "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"error\"}}");
  free(str);
}

TEST_F(DeviceJsonOtaUpdatesTest, setCalCfgResultWithSuccess) {
  TDS_DeviceCalCfgResult result = {};
  result.Command = SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE;

  DeviceJsonOtaUpdatesMock json_result;
  EXPECT_CALL(json_result, get_timestamp).WillRepeatedly(Return(1754227503));

  result.DataSize = sizeof(TCalCfg_FirmwareCheckResult);
  TCalCfg_FirmwareCheckResult *fw_result =
      (TCalCfg_FirmwareCheckResult *)result.Data;
  fw_result->Result = SUPLA_FIRMWARE_CHECK_RESULT_UPDATE_AVAILABLE;
  snprintf(fw_result->SoftVer, sizeof(fw_result->SoftVer), "%s", "1.1");
  snprintf(fw_result->ChangelogUrl, sizeof(fw_result->ChangelogUrl), "%s",
           "https://supla.org/changelog.html");
  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  char *str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"available\","
      "\"version\":\"1.1\",\"url\":\"https://supla.org/changelog.html\"}}");
  free(str);
}

TEST_F(DeviceJsonOtaUpdatesTest, stringOverflow) {
  TDS_DeviceCalCfgResult result = {};
  result.Command = SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE;

  DeviceJsonOtaUpdatesMock json_result;
  EXPECT_CALL(json_result, get_timestamp).WillRepeatedly(Return(1754227503));

  result.DataSize = sizeof(TCalCfg_FirmwareCheckResult);
  TCalCfg_FirmwareCheckResult *fw_result =
      (TCalCfg_FirmwareCheckResult *)result.Data;
  fw_result->Result = SUPLA_FIRMWARE_CHECK_RESULT_UPDATE_AVAILABLE;
  memset(fw_result->SoftVer, '1', sizeof(fw_result->SoftVer));
  memset(fw_result->ChangelogUrl, '2', sizeof(fw_result->ChangelogUrl));

  EXPECT_TRUE(json_result.set_calcfg_result(&result));

  char *str = json_result.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"otaUpdate\":{\"timestamp\":1754227503,\"status\":\"available\","
      "\"version\":\"11111111111111111111\",\"url\":"
      "\"2222222222222222222222222222222222222222222222222222222222222222222222"
      "222222222222222222222222222222\"}}");
  free(str);
}

TEST_F(DeviceJsonOtaUpdatesTest, merge) {
  DeviceJsonOtaUpdatesMock result1;

  result1.set_user_config("{\"x\":\"y\"}");

  result1.set_properties(
      "{\"a\":\"b\",\"otaUpdate\":{\"timestamp\":1754227503,\"status\":"
      "\"checking\"}}");

  DeviceJsonOtaUpdatesMock config2;
  config2.set_user_config("{\"c\":\"v\"}");
  config2.set_properties(
      "{\"otaUpdate\":{\"timestamp\":1754227600,\"status\":\"notAvailable\"}}");
  config2.merge(&result1);

  char *str = result1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{\"x\":\"y\"}");

  free(str);

  str = result1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"otaUpdate\":{\"timestamp\":1754227600,"
               "\"status\":\"notAvailable\"}}");

  free(str);
}

} /* namespace testing */
