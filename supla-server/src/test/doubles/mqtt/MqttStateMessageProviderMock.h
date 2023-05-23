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

#ifndef MQTTSTATEMESSAGEPROVIDERMOCK_H_
#define MQTTSTATEMESSAGEPROVIDERMOCK_H_

#include "gmock/gmock.h"  // NOLINT
#include "mqtt_abstract_state_message_provider.h"
#include "mqtt_db.h"

class MqttStateMessageProviderMock
    : public supla_mqtt_abstract_state_message_provider {
 public:
  MqttStateMessageProviderMock();
  virtual ~MqttStateMessageProviderMock();

  MOCK_METHOD0(_get_channel_property_getter,
               supla_abstract_channel_property_getter *());
  MOCK_METHOD0(_get_user_suid, const char *());
};

#endif /* MQTTSTATEMESSAGEPROVIDERMOCK_H_ */
