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

#include "MqttUnpublisherIntegrationTest.h"
#include "log.h"
#include "mqtt_unpublisher.h"
#include "mqtt_unpublisher_datasource.h"

namespace testing {

MqttUnpublisherIntegrationTest::MqttUnpublisherIntegrationTest()
    : MqttClientIntegrationTest() {}

MqttUnpublisherIntegrationTest::~MqttUnpublisherIntegrationTest() {}

supla_mqtt_client *MqttUnpublisherIntegrationTest::clientInit(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource) {
  return new supla_mqtt_unpublisher(library_adapter, settings, datasource);
}

supla_mqtt_client_datasource *MqttUnpublisherIntegrationTest::dsInit(
    supla_mqtt_client_settings *settings) {
  return new supla_mqtt_unpublisher_datasource(settings);
}

void MqttUnpublisherIntegrationTest::SetUp() {
  initTestDatabase();
  runSqlScript("DataForMqttTests.sql");

  MqttClientIntegrationTest::SetUp();
}

} /* namespace testing */
