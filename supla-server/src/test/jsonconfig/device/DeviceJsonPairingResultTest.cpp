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

#include "DeviceJsonPairingResultTest.h"

#include "doubles/jsonconfig/device/DeviceJsonOtaUpdatesMock.h"

namespace testing {

DeviceJsonPairingResultTest::DeviceJsonPairingResultTest(void) {}

DeviceJsonPairingResultTest::~DeviceJsonPairingResultTest(void) {}

void DeviceJsonPairingResultTest::SetUp() {
  Test::SetUp();
  ON_CALL(result, get_time).WillByDefault([](void) {
    std::tm t = {};
    t.tm_year = 2025 - 1900;
    t.tm_mon = 8 - 1;
    t.tm_mday = 25;
    t.tm_hour = 18;
    t.tm_min = 48;
    t.tm_sec = 0;

    return std::mktime(&t);
  });
}

TEST_F(DeviceJsonPairingResultTest, null) {
  TDS_DeviceCalCfgResult calcafg_result = {};

  EXPECT_EQ(result.calcfg_result_to_json(nullptr), nullptr);
  EXPECT_EQ(result.calcfg_result_to_json(&calcafg_result), nullptr);
}

TEST_F(DeviceJsonPairingResultTest, result0) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  calcafg_result.Command = SUPLA_CALCFG_CMD_START_SUBDEVICE_PAIRING;

  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str, "{\"time\":\"2025-08-25T18:48:00Z\",\"result\":\"CMD_RESULT_0\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, notSupported) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  calcafg_result.Command = SUPLA_CALCFG_CMD_START_SUBDEVICE_PAIRING;
  calcafg_result.Result = SUPLA_CALCFG_RESULT_NOT_SUPPORTED;

  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str, "{\"time\":\"2025-08-25T18:48:00Z\",\"result\":\"NOT_SUPPORTED\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, unauthorized) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  calcafg_result.Command = SUPLA_CALCFG_CMD_START_SUBDEVICE_PAIRING;
  calcafg_result.Result = SUPLA_CALCFG_RESULT_UNAUTHORIZED;

  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str, "{\"time\":\"2025-08-25T18:48:00Z\",\"result\":\"UNAUTHORIZED\"}");
  free(str);
}

void DeviceJsonPairingResultTest::set_pairing_result(
    TDS_DeviceCalCfgResult* calcafg_result, unsigned char pairing_result_code) {
  calcafg_result->Command = SUPLA_CALCFG_CMD_START_SUBDEVICE_PAIRING;
  calcafg_result->Result = SUPLA_CALCFG_RESULT_TRUE;

  TCalCfg_SubdevicePairingResult* pairing_result =
      (TCalCfg_SubdevicePairingResult*)calcafg_result->Data;
  calcafg_result->DataSize = sizeof(TCalCfg_SubdevicePairingResult);

  pairing_result->PairingResult = pairing_result_code;
  snprintf(pairing_result->Name, sizeof(pairing_result->Name), "%s", "XYZ");
  pairing_result->NameSize =
      strnlen(pairing_result->Name, sizeof(pairing_result->Name));
  pairing_result->MaximumDurationSec = 120;
  pairing_result->ElapsedTimeSec = 60;
}

TEST_F(DeviceJsonPairingResultTest, procedureStarted) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result,
                     SUPLA_CALCFG_PAIRINGRESULT_PROCEDURE_STARTED);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"PROCEDURE_STARTED\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, ongoing) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result, SUPLA_CALCFG_PAIRINGRESULT_ONGOING);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"ONGOING\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, noNewDeviceFound) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result,
                     SUPLA_CALCFG_PAIRINGRESULT_NO_NEW_DEVICE_FOUND);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"NO_NEW_DEVICE_FOUND\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, success) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result, SUPLA_CALCFG_PAIRINGRESULT_SUCCESS);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"SUCCESS\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, deviceNotSupported) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result,
                     SUPLA_CALCFG_PAIRINGRESULT_DEVICE_NOT_SUPPORTED);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"DEVICE_NOT_SUPPORTED\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, resourceLimitExceeded) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result,
                     SUPLA_CALCFG_PAIRINGRESULT_RESOURCES_LIMIT_EXCEEDED);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"RESOURCES_LIMIT_EXCEEDED\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, notStartedNotReady) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result,
                     SUPLA_CALCFG_PAIRINGRESULT_NOT_STARTED_NOT_READY);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"NOT_STARTED_NOT_READY\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, notStartedBusy) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result,
                     SUPLA_CALCFG_PAIRINGRESULT_NOT_STARTED_BUSY);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"NOT_STARTED_BUSY\"}");
  free(str);
}

TEST_F(DeviceJsonPairingResultTest, result234) {
  TDS_DeviceCalCfgResult calcafg_result = {};
  set_pairing_result(&calcafg_result, 234);
  char* str = result.calcfg_result_to_json(&calcafg_result);
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"time\":\"2025-08-25T18:48:00Z\",\"name\":\"XYZ\","
               "\"startedAt\":\"2025-08-25T18:47:00Z\",\"timeoutAt\":\"2025-08-"
               "25T18:49:00Z\",\"result\":\"RAIRING_RESULT_234\"}");
  free(str);
}

} /* namespace testing */
