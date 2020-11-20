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

#include "MqttPublisherIntegrationTest.h"

namespace testing {

MqttPublisherIntegrationTest::MqttPublisherIntegrationTest()
    : IntegrationTest() {}

MqttPublisherIntegrationTest::~MqttPublisherIntegrationTest() {}

void MqttPublisherIntegrationTest::SetUp() {
  initTestDatabase();
  runSqlScript("DataForMqttTests.sql");

  iniSettings = new supla_mqtt_client_ini_settings();
  libraryAdapter = new MqttClientLibraryAdapterMock(iniSettings);
  dataSource = new supla_mqtt_publisher_datasource(iniSettings);

  publisher = new supla_mqtt_publisher(libraryAdapter, iniSettings, dataSource);
  publisher->start();
}

void MqttPublisherIntegrationTest::TearDown() {
  publisher->stop();
  delete publisher;
  delete dataSource;
  delete iniSettings;
  delete libraryAdapter;
}

void MqttPublisherIntegrationTest::waitForConnection() {
  for (int a = 0; a < 5000; a++) {
    if (libraryAdapter->is_connected()) {
      return;
    }
    usleep(1000);
  }

  ASSERT_TRUE(false);
}

TEST_F(MqttPublisherIntegrationTest, allEvents) { waitForConnection(); }

} /* namespace testing */
