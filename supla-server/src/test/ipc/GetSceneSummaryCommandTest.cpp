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

#include "ipc/GetSceneSummaryCommandTest.h"

namespace testing {

void GetSceneSummaryCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetSceneSummaryCommandMock(socketAdapter);
}

void GetSceneSummaryCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetSceneSummaryCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetSceneSummaryCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetSceneSummaryCommandTest, defaultStateWithoutAssignment) {
  EXPECT_CALL(*cmd, get_scene_state(10, 20, NotNull())).WillOnce(Return(true));
  commandProcessingTest("GET-SCENE-SUMMARY:10,20\n", "SUMMARY:20,0,0,,0,0\n");
}

TEST_F(GetSceneSummaryCommandTest, sceneStarted) {
  EXPECT_CALL(*cmd, get_scene_state(10, 20, _))
      .WillOnce([](int user_id, int scene_id, supla_scene_state *state) {
        struct timeval started_at = {};
        gettimeofday(&started_at, NULL);
        *state = supla_scene_state(supla_caller(ctClient, 155, "iPhone Elon"),
                                   started_at, 200);
        return true;
      });

  snprintf(buffer, sizeof(buffer), "GET-SCENE-SUMMARY:10,20\n");

  int summary_scene_id = -1;
  int summary_initiator_type = -1;
  int summary_initiator_id = -1;
  char summary_initiator_name[100] = {};
  int sumamry_time_from_start = -1;
  int summary_time_left = -1;

  EXPECT_CALL(*socketAdapter, send_data(_))
      .WillOnce([&summary_scene_id, &summary_initiator_type,
                 &summary_initiator_id, &summary_initiator_name,
                 &sumamry_time_from_start,
                 &summary_time_left](const std::string &data) {
        sscanf(data.c_str(), "SUMMARY:%i,%i,%i,%[^,],%i,%i\n",
               &summary_scene_id, &summary_initiator_type,
               &summary_initiator_id, summary_initiator_name,
               &sumamry_time_from_start, &summary_time_left);
      });

  EXPECT_TRUE(getCommand()->process_command(
      buffer, sizeof(buffer), strnlen(buffer, IPC_BUFFER_MAX_SIZE)));

  EXPECT_EQ(summary_scene_id, 20);
  EXPECT_EQ(summary_initiator_type, 2);
  EXPECT_EQ(summary_initiator_id, 155);
  EXPECT_EQ(strncmp(summary_initiator_name,
                    "aVBob25lIEVsb24=", sizeof(summary_initiator_name)),
            0);
  EXPECT_GE(sumamry_time_from_start, 0);
  EXPECT_LE(summary_time_left, 200);
}

TEST_F(GetSceneSummaryCommandTest, getStateFailed) {
  EXPECT_CALL(*cmd, get_scene_state).WillOnce(Return(false));
  commandProcessingTest("GET-SCENE-SUMMARY:10,20\n", "UNKNOWN:20\n");
}

TEST_F(GetSceneSummaryCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_scene_state).Times(0);
  commandProcessingTest("GET-SCENE-SUMMARY:\n", "UNKNOWN:0\n");
}

TEST_F(GetSceneSummaryCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_scene_state).Times(0);
  commandProcessingTest("GET-SCENE-SUMMARY:0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetSceneSummaryCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_scene_state).Times(0);
  commandProcessingTest("GET-SCENE-SUMMARY:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
