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

#include "SetChannelFunctionIntegrationTest.h"
#include "log.h"

namespace testing {

SetChannelFunctionIntegrationTest::SetChannelFunctionIntegrationTest() {
  expectedResultCode = 0;
  expectedChannelID = 0;
  expectedFunction = 0;
  match = 0;
}

SetChannelFunctionIntegrationTest::~SetChannelFunctionIntegrationTest() {}

void SetChannelFunctionIntegrationTest::channelMatch(
    TSC_SetChannelFunctionResult *result, TSC_SuplaChannel_C *channel) {
  if (result) {
    ASSERT_EQ(result->ResultCode, expectedResultCode);
    ASSERT_EQ(result->ChannelID, expectedChannelID);
    ASSERT_EQ(result->Func, expectedFunction);
    match |= 0x1;
  } else if (channel && channel->Id == expectedChannelID) {
    if (channel->Func == expectedFunction) {
      match |= 0x2;
    }
  }

  if (match & 0x1 &&
      (match & 0x2 || expectedResultCode != SUPLA_RESULTCODE_TRUE)) {
    cancelIteration();
  }
}

void SetChannelFunctionIntegrationTest::onChannelFunctionSetResult(
    TSC_SetChannelFunctionResult *result) {
  ASSERT_FALSE(result == NULL);
  channelMatch(result, NULL);
}

void SetChannelFunctionIntegrationTest::channelUpdate(
    TSC_SuplaChannel_C *channel) {
  ASSERT_FALSE(channel == NULL);
  channelMatch(NULL, channel);
}

void SetChannelFunctionIntegrationTest::reconnect() {
  match = 0;
  GetChannelBasicCfg::reconnect();
}

TEST_F(SetChannelFunctionIntegrationTest,
       SetFunctionForNonexistentChannel_Unauthorized) {
  expectedResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  expectedChannelID = 100000;
  expectedFunction = SUPLA_CHANNELFNC_POWERSWITCH;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_set_channel_function(sclient, expectedChannelID,
                                              expectedFunction),
            0);

  iterateUntilDefaultTimeout();
}

TEST_F(SetChannelFunctionIntegrationTest, SetFunctionForNonexistentChannel) {
  superuserAuthorize();

  expectedResultCode = SUPLA_RESULTCODE_CHANNELNOTFOUND;
  expectedChannelID = 100000;
  expectedFunction = SUPLA_CHANNELFNC_POWERSWITCH;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_set_channel_function(sclient, expectedChannelID,
                                              expectedFunction),
            0);

  iterateUntilDefaultTimeout();
}

TEST_F(SetChannelFunctionIntegrationTest,
       SetFunctionForChannelWithNotAllowedType) {
  superuserAuthorize();

  expectedResultCode = SUPLA_RESULTCODE_NOT_ALLOWED;
  expectedChannelID = 301;
  expectedFunction = SUPLA_CHANNELFNC_POWERSWITCH;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_set_channel_function(sclient, expectedChannelID,
                                              expectedFunction),
            0);

  iterateUntilDefaultTimeout();
}

TEST_F(SetChannelFunctionIntegrationTest, SetAllowedFunction) {
  expectedResultCode = SUPLA_RESULTCODE_TRUE;
  expectedChannelID = 303;
  expectedFunction = SUPLA_CHANNELFNC_POWERSWITCH;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();
  getBasicCfg(expectedChannelID);

  ASSERT_EQ(channelBasicCfg.Func, SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER);

  ASSERT_GT(supla_client_set_channel_function(sclient, expectedChannelID,
                                              expectedFunction),
            0);

  iterateUntilDefaultTimeout();
  reconnect();
  getBasicCfg(expectedChannelID);
  ASSERT_EQ(channelBasicCfg.Func, expectedFunction);
}

TEST_F(SetChannelFunctionIntegrationTest, SetFunctionNone) {
  expectedResultCode = SUPLA_RESULTCODE_TRUE;
  expectedChannelID = 303;
  expectedFunction = SUPLA_CHANNELFNC_NONE;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();
  getBasicCfg(expectedChannelID);

  ASSERT_EQ(channelBasicCfg.Func, SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER);

  ASSERT_GT(supla_client_set_channel_function(sclient, expectedChannelID,
                                              expectedFunction),
            0);

  iterateUntilDefaultTimeout();
  reconnect();
  getBasicCfg(expectedChannelID);
  ASSERT_EQ(channelBasicCfg.Func, expectedFunction);
}

TEST_F(SetChannelFunctionIntegrationTest,
       SetTheFunctionForChannelBelongingToGroup) {
  runSqlScript("CreateChannelGroupWithChannelId303.sql");
  expectedResultCode = SUPLA_RESULTCODE_DENY_CHANNEL_BELONG_TO_GROUP;
  expectedChannelID = 303;
  expectedFunction = SUPLA_CHANNELFNC_POWERSWITCH;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();

  ASSERT_GT(supla_client_set_channel_function(sclient, expectedChannelID,
                                              expectedFunction),
            0);

  iterateUntilDefaultTimeout();
}

TEST_F(SetChannelFunctionIntegrationTest,
       SetTheFunctionForChannelWithSchedule) {
  runSqlScript("CreateScheduleForChannelId303.sql");
  expectedResultCode = SUPLA_RESULTCODE_DENY_CHANNEL_HAS_SCHEDULE;
  expectedChannelID = 303;
  expectedFunction = SUPLA_CHANNELFNC_POWERSWITCH;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();

  ASSERT_GT(supla_client_set_channel_function(sclient, expectedChannelID,
                                              expectedFunction),
            0);

  iterateUntilDefaultTimeout();
}

#ifndef SERVER_VERSION_23
TEST_F(SetChannelFunctionIntegrationTest,
       SetTheFunctionForChannelAssociatedWithSchedule) {
  runSqlScript("CreateSceneForChannelId303.sql");
  expectedResultCode = SUPLA_RESULTCODE_DENY_CHANNEL_IS_ASSOCIETED_WITH_SCENE;
  expectedChannelID = 303;
  expectedFunction = SUPLA_CHANNELFNC_POWERSWITCH;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();

  ASSERT_GT(supla_client_set_channel_function(sclient, expectedChannelID,
                                              expectedFunction),
            0);

  iterateUntilDefaultTimeout();
}
#else
#warning \
    "Delete the SERVER_VERSION_23 def. in v2.4 together with the ref. code."
#endif /*SERVER_VERSION_23*/

} /* namespace testing */
