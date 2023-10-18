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

#include "ipc/ActionCgCommandTest.h"

#include "actions/abstract_action_config.h"
#include "proto.h"

namespace testing {

void ActionCgCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  user = supla_user::find(10, true);
}

void ActionCgCommandTest::TearDown() { IpcCommandTest::TearDown(); }

supla_abstract_ipc_command *ActionCgCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(ActionCgCommandTest, noData) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_OPEN);
  cmd = &c;
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(ActionCgCommandTest, OpenWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_OPEN);
  cmd = &c;
  EXPECT_CALL(c, action_open_close(user, 30, true)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-OPEN:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, OpenWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_OPEN);
  cmd = &c;
  EXPECT_CALL(c, action_open_close).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-OPEN:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, OpenWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_OPEN);
  cmd = &c;
  EXPECT_CALL(c, action_open_close).Times(0);
  commandProcessingTest("ACTION-CG-OPEN:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, CloseWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_CLOSE);
  cmd = &c;
  EXPECT_CALL(c, action_open_close(user, 30, false)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-CLOSE:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, CloseWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_CLOSE);
  cmd = &c;
  EXPECT_CALL(c, action_open_close).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-CLOSE:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, CloseWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_CLOSE);
  cmd = &c;
  EXPECT_CALL(c, action_open_close).Times(0);
  commandProcessingTest("ACTION-CG-CLOSE:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, TurnOnWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TURN_ON);
  cmd = &c;
  EXPECT_CALL(c, action_turn_on(user, 30)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-TURN-ON:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, TurnOnWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TURN_ON);
  cmd = &c;
  EXPECT_CALL(c, action_turn_on).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-TURN-ON:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, TurnOnWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TURN_ON);
  cmd = &c;
  EXPECT_CALL(c, action_turn_on).Times(0);
  commandProcessingTest("ACTION-CG-TURN-ON:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, TurnOffWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TURN_OFF);
  cmd = &c;
  EXPECT_CALL(c, action_turn_off(user, 30)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-TURN-OFF:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, TurnOffWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TURN_OFF);
  cmd = &c;
  EXPECT_CALL(c, action_turn_off).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-TURN-OFF:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, TurnOffWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TURN_OFF);
  cmd = &c;
  EXPECT_CALL(c, action_turn_off).Times(0);
  commandProcessingTest("ACTION-CG-TURN-OFF:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, ToggleWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TOGGLE);
  cmd = &c;
  EXPECT_CALL(c, action_toggle(user, 30)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-TOGGLE:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, ToggleWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TOGGLE);
  cmd = &c;
  EXPECT_CALL(c, action_toggle).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-TOGGLE:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, ToggleWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_TOGGLE);
  cmd = &c;
  EXPECT_CALL(c, action_toggle).Times(0);
  commandProcessingTest("ACTION-CG-TOGGLE:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, StopWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_stop(user, 30)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-STOP:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, StopWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_stop).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-STOP:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, StopWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_stop).Times(0);
  commandProcessingTest("ACTION-CG-STOP:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, UpOrStopWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_UP_OR_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_up_or_stop(user, 30)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-UP-OR-STOP:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, UpOrStopWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_UP_OR_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_up_or_stop).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-UP-OR-STOP:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, UpOrStopWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_UP_OR_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_up_or_stop).Times(0);
  commandProcessingTest("ACTION-CG-UP-OR-STOP:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, DownOrStopWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_DOWN_OR_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_down_or_stop(user, 30)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-DOWN-OR-STOP:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, DownOrStopWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_DOWN_OR_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_down_or_stop).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-DOWN-OR-STOP:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, DownOrStopWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_DOWN_OR_STOP);
  cmd = &c;
  EXPECT_CALL(c, action_down_or_stop).Times(0);
  commandProcessingTest("ACTION-CG-DOWN-OR-STOP:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, StepByStepWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_STEP_BY_STEP);
  cmd = &c;
  EXPECT_CALL(c, action_step_by_step(user, 30)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-SBS:10,30\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, StepByStepWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_STEP_BY_STEP);
  cmd = &c;
  EXPECT_CALL(c, action_step_by_step).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-SBS:10,30\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, StepByStepWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_STEP_BY_STEP);
  cmd = &c;
  EXPECT_CALL(c, action_step_by_step).Times(0);
  commandProcessingTest("ACTION-CG-SBS:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, CopyWithSuccess) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_COPY);
  cmd = &c;
  EXPECT_CALL(c, action_copy(user, 30, 50, 100)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-COPY:10,30,50,100\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, CopyWithSuccessWithoidSourceDeviceId) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_COPY);
  cmd = &c;
  EXPECT_CALL(c, action_copy(user, 30, 0, 100)).WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-COPY:10,30,0,100\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, CopyWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_COPY);
  cmd = &c;
  EXPECT_CALL(c, action_copy).WillOnce(Return(false));
  commandProcessingTest("ACTION-CG-COPY:10,30,50,100\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, CopyWithoutParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_COPY);
  cmd = &c;
  EXPECT_CALL(c, action_copy).Times(0);
  commandProcessingTest("ACTION-CG-COPY:\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, paramsWithZeros) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_OPEN);
  cmd = &c;
  EXPECT_CALL(c, action_open_close).Times(0);
  commandProcessingTest("ACTION-CG-OPEN:0,0\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, badParams) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_OPEN);
  cmd = &c;
  EXPECT_CALL(c, action_open_close).Times(0);
  commandProcessingTest("ACTION-CG-OPEN:a,10\n", "UNKNOWN:0\n");
}

TEST_F(ActionCgCommandTest, ShutPartiallyWithDelta) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_SHUT_PARTIALLY);
  cmd = &c;
  EXPECT_CALL(c, action_shut(user, 30, Pointee(Eq(50)), true))
      .WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-SHUT-PARTIALLY:10,30,50,1\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, ShutPartiallyWithoutDelta) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_SHUT_PARTIALLY);
  cmd = &c;
  EXPECT_CALL(c, action_shut(user, 30, Pointee(Eq(50)), false))
      .WillOnce(Return(true));

  commandProcessingTest("ACTION-CG-SHUT-PARTIALLY:10,30,50,0\n", "OK:30\n");
}

TEST_F(ActionCgCommandTest, ShutPartiallyWithFilure) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_SHUT_PARTIALLY);
  cmd = &c;
  EXPECT_CALL(c, action_shut(user, 30, Pointee(Eq(50)), false))
      .WillOnce(Return(false));

  commandProcessingTest("ACTION-CG-SHUT-PARTIALLY:10,30,50,0\n", "FAIL:30\n");
}

TEST_F(ActionCgCommandTest, SetHvacParameters) {
  StrictMock<ActionCgCommandMock> c(socketAdapter, ACTION_HVAC_SET_PARAMETERS);
  cmd = &c;
  EXPECT_CALL(c, action_set_hvac_parameters(user, 30, NotNull()))
      .WillOnce([](supla_user *user, int group_id,
                   const supla_action_hvac_parameters *params) -> bool {
        EXPECT_EQ(params->get_duration_sec(), 1);
        EXPECT_EQ(params->get_mode(), 2);
        EXPECT_EQ(params->get_setpoint_temperature_heat(), 3);
        EXPECT_EQ(params->get_setpoint_temperature_cool(), 4);
        EXPECT_EQ(params->get_flags(), 5);
        return true;
      });

  commandProcessingTest("ACTION-CG-HVAC-SET-PARAMETERS:10,30,1,2,3,4,5\n",
                        "OK:30\n");
}

} /* namespace testing */
