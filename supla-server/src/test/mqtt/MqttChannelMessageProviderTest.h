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

#ifndef MQTTCHANNELMESSAGEPROVIDERTEST_H_
#define MQTTCHANNELMESSAGEPROVIDERTEST_H_

#include "MqttMessageProviderTest.h"
#include "gtest/gtest.h"  // NOLINT
#include "mqtt_channel_message_provider.h"
#include "mqtt_db.h"

namespace testing {

class MqttChannelMessageProviderTest : public MqttMessageProviderTest {
 protected:
  supla_mqtt_channel_message_provider *provider;
  void fillChannelData(_mqtt_db_data_row_channel_t *row_channel);
  void electricityMeterTest(int channel_flags);

 public:
  virtual void SetUp();
  virtual void TearDown();
  MqttChannelMessageProviderTest();
  virtual ~MqttChannelMessageProviderTest();
};

} /* namespace testing */

#endif /* MQTTCHANNELMESSAGEPROVIDERTEST_H_ */
