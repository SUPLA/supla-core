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
  ASSERT_GT(supla_client_set_channel_function(sclient, 100000,
                                              SUPLA_CHANNELFNC_POWERSWITCH),
            0);

  iterateUntilDefaultTimeout();
}

} /* namespace testing */
