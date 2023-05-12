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

#include "GetIcValueCommandTest.h"

namespace testing {

void GetIcValueCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetIcValueCommandMock(socketAdapter);
}

void GetIcValueCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetIcValueCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetIcValueCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetIcValueCommandTest, getIcmValueWithSuccess) {
  TDS_ImpulseCounter_Value ic_val = {};
  ic_val.counter = 10000;

  supla_channel_ic_extended_value *icv = new supla_channel_ic_extended_value(
      SUPLA_CHANNELFNC_IC_GAS_METER, &ic_val, "PLN", "", 500, 1000);

  EXPECT_CALL(*cmd, get_ic_extended_value(10, 20, 30)).WillOnce(Return(icv));

  commandProcessingTest("GET-IC-VALUE:10,20,30\n",
                        "VALUE:50,500,1000,10000,10000,PLN,bcKz\n");
}

TEST_F(GetIcValueCommandTest, getIcmValueWithFilure) {
  supla_channel_ic_extended_value *icv = NULL;
  EXPECT_CALL(*cmd, get_ic_extended_value).WillOnce(Return(icv));
  commandProcessingTest("GET-IC-VALUE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(GetIcValueCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_ic_extended_value).Times(0);
  commandProcessingTest("GET-IC-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(GetIcValueCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_ic_extended_value).Times(0);
  commandProcessingTest("GET-IC-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetIcValueCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_ic_extended_value).Times(0);
  commandProcessingTest("GET-IC-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
