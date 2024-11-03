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

#include "ipc/GetChannelStateCommandTest.h"

namespace testing {

void GetChannelStateCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetChannelStateCommandMock(socketAdapter);
}

void GetChannelStateCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetChannelStateCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetChannelStateCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetChannelStateCommandTest, getChannelStateWithSuccess) {
  EXPECT_CALL(*cmd, get_channel_state(NotNull(), 10, 20, 30, false, false))
      .WillOnce([](TDSC_ChannelState *state, int user_id, int device_id,
                   int channel_id, bool send_request,
                   bool get_from_extended_value) {
        state->Fields = 0xFFFFFFFF;
        state->defaultIconField = 8;
        state->SwitchCycleCount = 15;
        state->IPv4 = 1593737281;
        state->MAC[0] = 0xAA;
        state->MAC[1] = 0xBB;
        state->MAC[2] = 0xCC;
        state->MAC[3] = 0xDD;
        state->MAC[4] = 0xEE;
        state->MAC[5] = 0xFF;
        state->BatteryLevel = 88;
        state->BatteryPowered = 1;
        state->WiFiRSSI = 45;
        state->WiFiSignalStrength = 80;
        state->BridgeNodeOnline = 1;
        state->BridgeNodeSignalStrength = 12;
        state->Uptime = 1000;
        state->ConnectionUptime = 999;
        state->BatteryHealth = 123;
        state->LastConnectionResetCause = 2;
        state->LightSourceLifespan = 5;
        state->LightSourceLifespanLeft = -10;
        state->LightSourceOperatingTime = -11;
        state->OperatingTime = 400;

        return true;
      });

  commandProcessingTest("GET-CHANNEL-STATE:10,20,30,0,0\n",
                        "STATE:30,,15,94.254.128.65,AA:BB:CC:DD:EE:FF,88,1,45,"
                        "80,1,12,1000,999,123,2,,,,\n");
}

TEST_F(GetChannelStateCommandTest,
       getChannelStateWithSuccess_LightSourceLifespan) {
  EXPECT_CALL(*cmd, get_channel_state(NotNull(), 10, 20, 30, false, false))
      .WillOnce([](TDSC_ChannelState *state, int user_id, int device_id,
                   int channel_id, bool send_request,
                   bool get_from_extended_value) {
        state->Fields = SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN;
        state->LightSourceLifespan = 5;
        state->LightSourceLifespanLeft = -10;
        return true;
      });

  commandProcessingTest("GET-CHANNEL-STATE:10,20,30,0,0\n",
                        "STATE:30,0,,,,,,,,,,,,,,5,-10,,\n");
}

TEST_F(GetChannelStateCommandTest,
       getChannelStateWithSuccess_LightSourceOperatingTime) {
  EXPECT_CALL(*cmd, get_channel_state(NotNull(), 10, 20, 30, false, false))
      .WillOnce([](TDSC_ChannelState *state, int user_id, int device_id,
                   int channel_id, bool send_request,
                   bool get_from_extended_value) {
        state->Fields = SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME;
        state->LightSourceOperatingTime = -11;
        return true;
      });

  commandProcessingTest("GET-CHANNEL-STATE:10,20,30,0,0\n",
                        "STATE:30,0,,,,,,,,,,,,,,,,-11,\n");
}

TEST_F(GetChannelStateCommandTest, getChannelStateWithSuccess_OperatingTime) {
  EXPECT_CALL(*cmd, get_channel_state(NotNull(), 10, 20, 30, false, false))
      .WillOnce([](TDSC_ChannelState *state, int user_id, int device_id,
                   int channel_id, bool send_request,
                   bool get_from_extended_value) {
        state->Fields = SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME;
        state->OperatingTime = 400;
        return true;
      });

  commandProcessingTest("GET-CHANNEL-STATE:10,20,30,0,0\n",
                        "STATE:30,0,,,,,,,,,,,,,,,,,400\n");
}

TEST_F(GetChannelStateCommandTest, send_request) {
  EXPECT_CALL(*cmd, get_channel_state(NotNull(), 10, 20, 30, true, false))
      .WillOnce(Return(true));

  commandProcessingTest("GET-CHANNEL-STATE:10,20,30,1,0\n",
                        "STATE:30,0,,,,,,,,,,,,,,,,,\n");
}

TEST_F(GetChannelStateCommandTest, get_from_extended_value) {
  EXPECT_CALL(*cmd, get_channel_state(NotNull(), 10, 20, 30, false, true))
      .WillOnce(Return(true));

  commandProcessingTest("GET-CHANNEL-STATE:10,20,30,0,1\n",
                        "STATE:30,0,,,,,,,,,,,,,,,,,\n");
}

TEST_F(GetChannelStateCommandTest, getChannelStateWithFilure) {
  EXPECT_CALL(*cmd, get_channel_state).WillOnce(Return(false));
  commandProcessingTest("GET-CHANNEL-STATE:10,20,30,0,0\n",
                        "NOT-AVAILABLE:30\n");
}

TEST_F(GetChannelStateCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_channel_state).Times(0);
  commandProcessingTest("GET-CHANNEL-STATE:\n", "UNKNOWN:0\n");
}

TEST_F(GetChannelStateCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_channel_state).Times(0);
  commandProcessingTest("GET-CHANNEL-STATE:0,0,0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetChannelStateCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_channel_state).Times(0);
  commandProcessingTest("GET-CHANNEL-STATE:a,10,c,0,0\n", "UNKNOWN:0\n");
}

} /* namespace testing */
