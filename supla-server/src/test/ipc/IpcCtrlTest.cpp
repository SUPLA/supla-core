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

namespace testing {

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
  get_char_cmd = new GetCharCommandMock();
  ipc_ctrl->add_command(get_char_cmd);
}

void IpcCtrlTest::TearDown() {
  Test::TearDown();
  delete ipc_ctrl;
  delete socket_adapter;
}

TEST_F(IpcCtrlTest, timeout) {
  struct timeval then = {};
  gettimeofday(&then, NULL);

  EXPECT_FALSE(ipc_ctrl->is_timeout());

  EXPECT_CALL(*ipc_ctrl, is_terminated()).WillRepeatedly(Return(false));
  EXPECT_CALL(*ipc_ctrl, terminate()).Times(1);
  EXPECT_CALL(*socket_adapter, send(std::string("SUPLA SERVER CTRL\n")))
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
  EXPECT_CALL(*socket_adapter, send(std::string("SUPLA SERVER CTRL\n")));
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
  EXPECT_CALL(*socket_adapter, send(std::string("SUPLA SERVER CTRL\n")));
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
  char recv_buffer[] = "GET-CHAR-VALUE:1,2,3\n";
  socket_adapter->set_recv_buffer(recv_buffer, sizeof(recv_buffer));

  EXPECT_CALL(*ipc_ctrl, is_terminated()).WillRepeatedly(Return(false));
  EXPECT_CALL(*socket_adapter, is_error()).WillRepeatedly(Return(false));
  EXPECT_CALL(*socket_adapter, send(std::string("SUPLA SERVER CTRL\n")))
      .Times(1);

  ipc_ctrl->execute();
}

} /* namespace testing */
