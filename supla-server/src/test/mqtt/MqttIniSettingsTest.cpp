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

#include "MqttIniSettingsTest.h"

namespace testing {

MqttIniSettingsTest::MqttIniSettingsTest(void) {}

MqttIniSettingsTest::~MqttIniSettingsTest(void) {}

void MqttIniSettingsTest::SetUp() {
  iniSettings = new supla_mqtt_client_ini_settings();
}

void MqttIniSettingsTest::TearDown() { delete iniSettings; }

TEST_F(MqttIniSettingsTest, serverConfigurationTest) {
  ASSERT_EQ(strcmp(iniSettings->getHost(), "192.168.111.30"), 0);
  ASSERT_EQ(iniSettings->getPort(), 1883);
  ASSERT_EQ(strcmp(iniSettings->getUsername(), "supla-server"), 0);
  ASSERT_EQ(strcmp(iniSettings->getPassword(), "alkdmalsdaskdn"), 0);
  ASSERT_EQ(strcmp(iniSettings->getPrefix(), "supla/%email%"), 0);
  ASSERT_EQ(iniSettings->isMQTTEnabled(), true);
  ASSERT_EQ(iniSettings->isSSLEnabled(), false);

  char clientId[CLIENTID_MAX_SIZE];
  memset(clientId, 0, CLIENTID_MAX_SIZE);
  iniSettings->getClientId(clientId, CLIENTID_MAX_SIZE, "test");

  ASSERT_EQ(strcmp(clientId, "NunYnx-test"), 0);
  ASSERT_EQ(iniSettings->getKeepAlive(), 30);
}

} /* namespace testing */
