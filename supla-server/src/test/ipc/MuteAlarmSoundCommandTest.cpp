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

#include "ipc/MuteAlarmSoundCommandTest.h"

namespace testing {

void MuteAlarmSoundCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new MuteAlarmSoundCommandMock(socketAdapter);
}

void MuteAlarmSoundCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *MuteAlarmSoundCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(MuteAlarmSoundCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(MuteAlarmSoundCommandTest, TakeOcrPhotoWithSuccess) {
  EXPECT_CALL(*cmd, mute_alarm_sound(10, 20, 30)).WillOnce(Return(true));

  commandProcessingTest("MUTE-ALARM-SOUND:10,20,30\n", "OK:30\n");
}

TEST_F(MuteAlarmSoundCommandTest, TakeOcrPhotoWithFilure) {
  EXPECT_CALL(*cmd, mute_alarm_sound).WillOnce(Return(false));
  commandProcessingTest("MUTE-ALARM-SOUND:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(MuteAlarmSoundCommandTest, noParams) {
  EXPECT_CALL(*cmd, mute_alarm_sound).Times(0);
  commandProcessingTest("MUTE-ALARM-SOUND:\n", "UNKNOWN:0\n");
}

TEST_F(MuteAlarmSoundCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, mute_alarm_sound).Times(0);
  commandProcessingTest("MUTE-ALARM-SOUND:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(MuteAlarmSoundCommandTest, badParams) {
  EXPECT_CALL(*cmd, mute_alarm_sound).Times(0);
  commandProcessingTest("MUTE-ALARM-SOUND:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
