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

#include <ipc/OnMqttSettingsChangedCommandTest.h>

namespace testing {

void OnMqttSettingsChangedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OnMqttSettingsChangedCommandMock(socketAdapter);
}

void OnMqttSettingsChangedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OnMqttSettingsChangedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OnMqttSettingsChangedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OnMqttSettingsChangedCommandTest, stateWebhookChanged) {
  EXPECT_CALL(*cmd, on_mqtt_settings_changed(10));

  commandProcessingTest("USER-ON-MQTT-SETTINGS-CHANGED:10\n", "OK:10\n");
}

TEST_F(OnMqttSettingsChangedCommandTest, noParams) {
  EXPECT_CALL(*cmd, on_mqtt_settings_changed).Times(0);
  commandProcessingTest("USER-ON-MQTT-SETTINGS-CHANGED:\n", "UNKNOWN:0\n");
}

TEST_F(OnMqttSettingsChangedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, on_mqtt_settings_changed).Times(0);
  commandProcessingTest("USER-ON-MQTT-SETTINGS-CHANGED:0\n", "UNKNOWN:0\n");
}

} /* namespace testing */
