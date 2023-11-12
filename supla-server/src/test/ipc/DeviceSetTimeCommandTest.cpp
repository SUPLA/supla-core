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

#include "ipc/DeviceSetTimeCommandTest.h"

namespace testing {

void DeviceSetTimeCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new DeviceSetTimeCommandMock(socketAdapter);
  user = supla_user::find(10, true);
}

void DeviceSetTimeCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *DeviceSetTimeCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(DeviceSetTimeCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(DeviceSetTimeCommandTest, setTimeWithFilure) {
  EXPECT_CALL(*cmd, set_time).WillOnce(Return(false));
  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "FAIL:20\n");
}

TEST_F(DeviceSetTimeCommandTest, noParams) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest("DEVICE-SET-TIME:\n", "UNKNOWN:0\n");
}

TEST_F(DeviceSetTimeCommandTest, onlyUserId) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest("DEVICE-SET-TIME:5\n", "UNKNOWN:5\n");
}

TEST_F(DeviceSetTimeCommandTest, yearOutOfRange) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,1899,11,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,3001,11,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");
}

TEST_F(DeviceSetTimeCommandTest, yearMinMax) {
  EXPECT_CALL(*cmd, set_time(user, 20, 1900, 11, 3, 6, 9, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  EXPECT_CALL(*cmd, set_time(user, 20, 3000, 11, 3, 6, 9, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,1900,11,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,3000,11,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");
}

TEST_F(DeviceSetTimeCommandTest, monthOutOfRange) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,0,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,13,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");
}

TEST_F(DeviceSetTimeCommandTest, monthMinMax) {
  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 1, 3, 6, 9, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 12, 3, 6, 9, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,1,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,12,3,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");
}

TEST_F(DeviceSetTimeCommandTest, dayOutOfRange) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,0,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,32,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");
}

TEST_F(DeviceSetTimeCommandTest, dayMinMax) {
  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 1, 6, 9, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 31, 6, 9, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,1,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,31,6,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");
}

TEST_F(DeviceSetTimeCommandTest, dayOfWeekOutOfRange) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,0,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,8,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");
}

TEST_F(DeviceSetTimeCommandTest, dayOfWeekMinMax) {
  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 3, 1, 9, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 3, 7, 9, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,1,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,7,9,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");
}

TEST_F(DeviceSetTimeCommandTest, hourOutOfRange) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,-1,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,24,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");
}

TEST_F(DeviceSetTimeCommandTest, hourMinMax) {
  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 3, 6, 1, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 3, 6, 23, 39, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,1,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,23,39,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");
}

TEST_F(DeviceSetTimeCommandTest, minuteOutOfRange) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,-1,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,60,15,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");
}

TEST_F(DeviceSetTimeCommandTest, minuteMinMax) {
  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 3, 6, 9, 0, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 3, 6, 9, 59, 15,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,0,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,59,15,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");
}

TEST_F(DeviceSetTimeCommandTest, secondOutOfRange) {
  EXPECT_CALL(*cmd, set_time).Times(0);
  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,39,-1,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,39,60,RXVyb3BlL1dhcnNhdw==\n",
      "UNKNOWN:10\n");
}

TEST_F(DeviceSetTimeCommandTest, secondMinMax) {
  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 3, 6, 9, 39, 0,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  EXPECT_CALL(*cmd, set_time(user, 20, 2023, 11, 3, 6, 9, 39, 59,
                             StrEq("Europe/Warsaw")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,39,0,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");

  commandProcessingTest(
      "DEVICE-SET-TIME:10,20,2023,11,3,6,9,39,59,RXVyb3BlL1dhcnNhdw==\n",
      "OK:20\n");
}

} /* namespace testing */
