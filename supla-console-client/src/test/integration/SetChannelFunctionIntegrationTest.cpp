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

namespace testing {

SetChannelFunctionIntegrationTest::SetChannelFunctionIntegrationTest() {
  expectedResultCode = 0;
  expectedChannelID = 0;
  expectedFunction = 0;
}

SetChannelFunctionIntegrationTest::~SetChannelFunctionIntegrationTest() {}

void SetChannelFunctionIntegrationTest::onChannelFunctionSetResult(
    TSC_SetChannelFunctionResult *result) {
  ASSERT_FALSE(result == NULL);
  ASSERT_EQ(result->ResultCode, expectedResultCode);
  ASSERT_EQ(result->ChannelID, expectedChannelID);
  ASSERT_EQ(result->Func, expectedFunction);
  cancelIteration();
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
  expectedChannelID = 303;
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

} /* namespace testing */
