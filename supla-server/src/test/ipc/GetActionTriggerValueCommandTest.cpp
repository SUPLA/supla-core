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

#include "ipc/GetActionTriggerValueCommandTest.h"

namespace testing {

void GetActionTriggerValueCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetActionTriggerValueCommandMock(socketAdapter);
}

void GetActionTriggerValueCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetActionTriggerValueCommandTest::getCommand(
    void) {
  return cmd;
}

TEST_F(GetActionTriggerValueCommandTest, getValueWithSuccess) {
  EXPECT_CALL(*cmd, get_channel_action_trigger_value(10, 20, 30, _))
      .WillOnce([](int user_id, int device_id, int channel_id,
                   TActionTriggerProperties *value) {
        value->ButtonMode = SUPLA_BUTTON_MODE_LOCKED;
        value->Flags = SUPLA_ACTION_TRIGGER_FLAG_WEEKLY_SCHEDULE_ENABLED;
        return true;
      });

  commandProcessingTest("GET-ACTION-TRIGGER-VALUE:10,20,30\n", "VALUE:1,1\n");
}

TEST_F(GetActionTriggerValueCommandTest, getValueWithFailure) {
  EXPECT_CALL(*cmd, get_channel_action_trigger_value)
      .WillOnce(Return(false));
  commandProcessingTest("GET-ACTION-TRIGGER-VALUE:10,20,30\n",
                        "UNKNOWN:30\n");
}

TEST_F(GetActionTriggerValueCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_channel_action_trigger_value).Times(0);
  commandProcessingTest("GET-ACTION-TRIGGER-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

}  // namespace testing
