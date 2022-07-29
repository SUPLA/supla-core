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

#include "ipc/GetEmValueCommandTest.h"

namespace testing {

void GetEmValueCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetEmValueCommandMock(socketAdapter);
}

void GetEmValueCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetEmValueCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetEmValueCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetEmValueCommandTest, getEmValueWithSuccess) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};

  em_ev.measured_values = EM_VAR_ALL;
  em_ev.m_count = 1;
  em_ev.m[0].freq = 1;
  em_ev.m[0].voltage[0] = 2;
  em_ev.m[0].voltage[1] = 3;
  em_ev.m[0].voltage[2] = 4;
  em_ev.m[0].current[0] = 5;
  em_ev.m[0].current[1] = 6;
  em_ev.m[0].current[2] = 7;
  em_ev.m[0].power_active[0] = 8;
  em_ev.m[0].power_active[1] = 9;
  em_ev.m[0].power_active[2] = 10;
  em_ev.m[0].power_reactive[0] = 11;
  em_ev.m[0].power_reactive[1] = 12;
  em_ev.m[0].power_reactive[2] = 13;
  em_ev.m[0].power_apparent[0] = 14;
  em_ev.m[0].power_apparent[1] = 15;
  em_ev.m[0].power_apparent[2] = 16;
  em_ev.m[0].power_factor[0] = 17;
  em_ev.m[0].power_factor[1] = 18;
  em_ev.m[0].power_factor[2] = 19;
  em_ev.m[0].phase_angle[0] = 20;
  em_ev.m[0].phase_angle[1] = 21;
  em_ev.m[0].phase_angle[2] = 22;
  em_ev.total_forward_active_energy[0] = 2300000;
  em_ev.total_forward_active_energy[1] = 2400000;
  em_ev.total_forward_active_energy[2] = 2500000;
  em_ev.total_reverse_active_energy[0] = 26;
  em_ev.total_reverse_active_energy[1] = 27;
  em_ev.total_reverse_active_energy[2] = 28;
  em_ev.total_forward_reactive_energy[0] = 29;
  em_ev.total_forward_reactive_energy[1] = 30;
  em_ev.total_forward_reactive_energy[2] = 31;
  em_ev.total_reverse_reactive_energy[0] = 32;
  em_ev.total_reverse_reactive_energy[1] = 33;
  em_ev.total_reverse_reactive_energy[2] = 34;

  supla_channel_electricity_measurement *em =
      new supla_channel_electricity_measurement(30, &em_ev, 50, "PLN");

  EXPECT_CALL(*cmd, get_electricity_measurement(10, 20, 30))
      .WillOnce(Return(em));

  commandProcessingTest(
      "GET-EM-VALUE:10,20,30\n",
      "VALUE:65535,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,"
      "2300000,2400000,2500000,26,27,28,29,30,31,32,33,34,36,50,PLN\n");
}

TEST_F(GetEmValueCommandTest, getEmValueWithFilure) {
  supla_channel_electricity_measurement *em = NULL;
  EXPECT_CALL(*cmd, get_electricity_measurement).WillOnce(Return(em));
  commandProcessingTest("GET-EM-VALUE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(GetEmValueCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_electricity_measurement).Times(0);
  commandProcessingTest("GET-EM-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(GetEmValueCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_electricity_measurement).Times(0);
  commandProcessingTest("GET-EM-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetEmValueCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_electricity_measurement).Times(0);
  commandProcessingTest("GET-EM-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
