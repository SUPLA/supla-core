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

#ifndef MQTTSTATEMESSAGEPROVIDERTEST_H_
#define MQTTSTATEMESSAGEPROVIDERTEST_H_

#include <string>

#include "MqttMessageProviderTest.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "doubles/mqtt/MqttStateMessageProviderMock.h"
#include "gtest/gtest.h"  // NOLINT

namespace testing {

class MqttStateMessageProviderTest : public MqttMessageProviderTest {
 protected:
  std::string short_unique_id;
  MqttStateMessageProviderMock provider;
  ChannelPropertyGetterMock propertyGetter;
  void SetResultValue(int func, bool connected,
                      supla_abstract_channel_value *value);
  void SetIcMeasurementResult(void);
  void SetElectricityMeasurementResult(int measured_values, int m_count);

 public:
  virtual void SetUp();
};

} /* namespace testing */

#endif /* MQTTSTATEMESSAGEPROVIDERTEST_H_ */
