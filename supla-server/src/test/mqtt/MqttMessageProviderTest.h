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

#ifndef MQTTMESSAGEPROVIDERTEST_H_
#define MQTTMESSAGEPROVIDERTEST_H_

#include "gtest/gtest.h"  // NOLINT
#include "mqtt_user_message_provider.h"

namespace testing {

class MqttMessageProviderTest : public Test {
 protected:
  bool dataExists(supla_mqtt_message_provider *provider);
  bool fetchAndCompare(supla_mqtt_message_provider *provider,
                       const char *prefix, const char *topic_name,
                       const char *message, bool include_zero_byte);

 public:
  MqttMessageProviderTest();
  virtual ~MqttMessageProviderTest();
};

} /* namespace testing */

#endif /* MQTTMESSAGEPROVIDERTEST_H_ */
