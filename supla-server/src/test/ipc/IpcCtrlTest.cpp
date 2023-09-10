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

#include "ipc/IpcCtrlTest.h"

#include "ipc/ipc_ctrl.h"

namespace testing {

using std::list;
using std::set;
using std::set_difference;
using std::sort;
using std::string;
using std::vector;

IpcCtrlTest::IpcCtrlTest() : Test() {
  socket_adapter = NULL;
  ipc_ctrl = NULL;
  get_char_cmd = NULL;
}

IpcCtrlTest::~IpcCtrlTest() {}

void IpcCtrlTest::SetUp() {
  Test::SetUp();
  socket_adapter = new IpcSocketAdapterMock(-1);
  ipc_ctrl = new IpcCtrlMock(socket_adapter);
  get_char_cmd = new GetCharCommandMock(socket_adapter);
  ipc_ctrl->add_command(get_char_cmd);
}

void IpcCtrlTest::TearDown() {
  Test::TearDown();
  delete ipc_ctrl;
}

TEST_F(IpcCtrlTest, timeout) {
  struct timeval then = {};
  gettimeofday(&then, NULL);

  EXPECT_FALSE(ipc_ctrl->is_timeout());

  EXPECT_CALL(*ipc_ctrl, is_terminated()).WillRepeatedly(Return(false));
  EXPECT_CALL(*ipc_ctrl, terminate()).Times(1);
  EXPECT_CALL(*socket_adapter, send_data(string("SUPLA SERVER CTRL\n")))
      .Times(1);
  EXPECT_CALL(*socket_adapter, is_error()).WillRepeatedly(Return(false));

  ipc_ctrl->set_timeout(2);
  ipc_ctrl->execute();

  struct timeval now = {};
  gettimeofday(&now, NULL);

  EXPECT_GE(now.tv_sec - then.tv_sec, 2);
  EXPECT_LE(now.tv_sec - then.tv_sec, 3);
  EXPECT_TRUE(ipc_ctrl->is_timeout());
}

TEST_F(IpcCtrlTest, terminate) {
  struct timeval then = {};
  gettimeofday(&then, NULL);

  EXPECT_CALL(*ipc_ctrl, is_terminated()).WillRepeatedly(Return(true));
  EXPECT_CALL(*socket_adapter, send_data(string("SUPLA SERVER CTRL\n")));
  EXPECT_CALL(*socket_adapter, is_error()).WillRepeatedly(Return(false));
  ipc_ctrl->set_timeout(10);
  ipc_ctrl->execute();

  struct timeval now = {};
  gettimeofday(&now, NULL);

  EXPECT_GE(now.tv_sec - then.tv_sec, 0);
  EXPECT_LE(now.tv_sec - then.tv_sec, 1);

  EXPECT_TRUE(ipc_ctrl->is_terminated());
  EXPECT_FALSE(ipc_ctrl->is_timeout());
}

TEST_F(IpcCtrlTest, adapterError) {
  struct timeval then = {};
  gettimeofday(&then, NULL);

  EXPECT_CALL(*ipc_ctrl, is_terminated()).WillRepeatedly(Return(false));
  EXPECT_CALL(*ipc_ctrl, terminate()).Times(1);
  EXPECT_CALL(*socket_adapter, send_data(string("SUPLA SERVER CTRL\n")));
  EXPECT_CALL(*socket_adapter, is_error()).WillRepeatedly(Return(true));
  ipc_ctrl->set_timeout(10);
  ipc_ctrl->execute();

  struct timeval now = {};
  gettimeofday(&now, NULL);

  EXPECT_GE(now.tv_sec - then.tv_sec, 0);
  EXPECT_LE(now.tv_sec - then.tv_sec, 1);

  EXPECT_FALSE(ipc_ctrl->is_terminated());
  EXPECT_FALSE(ipc_ctrl->is_timeout());
}

TEST_F(IpcCtrlTest, singleCommand) {
  struct timeval then = {};
  gettimeofday(&then, NULL);

  char recv_buffer[] = "GET-CHAR-VALUE:12,23,34\n";
  socket_adapter->set_recv_buffer(recv_buffer, sizeof(recv_buffer));

  Sequence s1;

  EXPECT_CALL(*ipc_ctrl, is_terminated()).WillRepeatedly(Return(false));
  EXPECT_CALL(*socket_adapter, is_error()).WillRepeatedly(Return(false));

  EXPECT_CALL(*get_char_cmd, get_channel_char_value(12, 23, 34, _))
      .WillOnce([](int user_id, int device_id, int channel_id, char *value) {
        *value = 31;
        return true;
      });

  EXPECT_CALL(*socket_adapter, send_data(string("SUPLA SERVER CTRL\n")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(*socket_adapter, send_data(string("VALUE:31\n")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(*socket_adapter, is_error())
      .InSequence(s1)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*ipc_ctrl, terminate()).Times(1).InSequence(s1);

  ipc_ctrl->execute();

  struct timeval now = {};
  gettimeofday(&now, NULL);

  EXPECT_GE(now.tv_sec - then.tv_sec, 0);
  EXPECT_LE(now.tv_sec - then.tv_sec, 1);

  EXPECT_FALSE(ipc_ctrl->is_timeout());
}

TEST_F(IpcCtrlTest, multipleCommands) {
  struct timeval then = {};
  gettimeofday(&then, NULL);

  char recv_buffer[] =
      "GET-CHAR-VALUE:12,23,34\nGET-CHAR-VALUE:116,1163,2284\nGET-CHAR-VALUE:"
      "56,63,84\n";
  socket_adapter->set_recv_buffer(recv_buffer, sizeof(recv_buffer));

  Sequence s1;

  EXPECT_CALL(*ipc_ctrl, is_terminated()).WillRepeatedly(Return(false));
  EXPECT_CALL(*socket_adapter, is_error()).WillRepeatedly(Return(false));

  EXPECT_CALL(*get_char_cmd, get_channel_char_value(12, 23, 34, _))
      .WillOnce([](int user_id, int device_id, int channel_id, char *value) {
        *value = 66;
        return true;
      });

  EXPECT_CALL(*get_char_cmd, get_channel_char_value(116, 1163, 2284, _))
      .WillOnce([](int user_id, int device_id, int channel_id, char *value) {
        return false;
      });

  EXPECT_CALL(*get_char_cmd, get_channel_char_value(56, 63, 84, _))
      .WillOnce([](int user_id, int device_id, int channel_id, char *value) {
        *value = 1;
        return true;
      });

  EXPECT_CALL(*socket_adapter, send_data(string("SUPLA SERVER CTRL\n")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(*socket_adapter, send_data(string("VALUE:66\n")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(*socket_adapter, send_data(string("UNKNOWN:2284\n")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(*socket_adapter, send_data(string("VALUE:1\n")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(*socket_adapter, is_error())
      .InSequence(s1)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*ipc_ctrl, terminate()).Times(1).InSequence(s1);

  ipc_ctrl->execute();

  struct timeval now = {};
  gettimeofday(&now, NULL);

  EXPECT_GE(now.tv_sec - then.tv_sec, 0);
  EXPECT_LE(now.tv_sec - then.tv_sec, 1);

  EXPECT_FALSE(ipc_ctrl->is_timeout());
}

TEST_F(IpcCtrlTest, unknownCommand) {
  struct timeval then = {};
  gettimeofday(&then, NULL);

  char recv_buffer[] = "XYZ\n";
  socket_adapter->set_recv_buffer(recv_buffer, sizeof(recv_buffer));

  Sequence s1;

  EXPECT_CALL(*ipc_ctrl, is_terminated()).WillRepeatedly(Return(false));
  EXPECT_CALL(*socket_adapter, is_error()).WillRepeatedly(Return(false));

  EXPECT_CALL(*socket_adapter, send_data(string("SUPLA SERVER CTRL\n")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(*socket_adapter, send_data(string("COMMAND_UNKNOWN\n")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(*socket_adapter, is_error())
      .InSequence(s1)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*ipc_ctrl, terminate()).Times(2);

  ipc_ctrl->execute();

  struct timeval now = {};
  gettimeofday(&now, NULL);

  EXPECT_GE(now.tv_sec - then.tv_sec, 0);
  EXPECT_LE(now.tv_sec - then.tv_sec, 1);

  EXPECT_FALSE(ipc_ctrl->is_timeout());
}

TEST_F(IpcCtrlTest, thereShouldBeNoDuplicates) {
  supla_ipc_ctrl ipc(new IpcSocketAdapterMock(-1));

  vector<string> cmd_list = ipc.get_command_list();

  list<string> diff;

  sort(cmd_list.begin(), cmd_list.end());
  set<string> ucmd_list(cmd_list.begin(),
                        cmd_list.end());  // Sort and remove duplicates
  set_difference(cmd_list.begin(), cmd_list.end(), ucmd_list.begin(),
                 ucmd_list.end(), back_inserter(diff));

  EXPECT_EQ(diff.size(), 0);
}

TEST_F(IpcCtrlTest, checkTheNumberOfCommands) {
  supla_ipc_ctrl ipc(new IpcSocketAdapterMock(-1));
  EXPECT_EQ(ipc.get_command_list().size(), 61);
}

} /* namespace testing */
