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

#ifndef MqttPublisherIntegrationTest_H_
#define MqttPublisherIntegrationTest_H_

#include "MqttClientIntegrationTest.h"

namespace testing {

class MqttPublisherIntegrationTest : public MqttClientIntegrationTest {
 protected:
  virtual supla_mqtt_client *clientInit(
      supla_mqtt_client_library_adapter *library_adapter,
      supla_mqtt_client_settings *settings,
      supla_mqtt_client_datasource *datasource);
  virtual supla_mqtt_client_datasource *dsInit(
      supla_mqtt_client_settings *settings);
  void waitForActions(int expectedTopicCount);

 public:
  virtual void SetUp();
  MqttPublisherIntegrationTest();
  virtual ~MqttPublisherIntegrationTest();
};

} /* namespace testing */

#endif /* MqttPublisherIntegrationTest_H_ */
