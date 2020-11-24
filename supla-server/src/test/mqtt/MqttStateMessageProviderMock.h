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

#include "gtest/gtest.h"  // NOLINT
#include "mqtt_db.h"
#include "mqtt_state_message_provider_abstract.h"

class MqttStateMessageProviderMock
    : public supla_mqtt_state_message_provider_abstract {
 protected:
  virtual channel_complex_value *_get_complex_value(int user_id, int device_id,
                                                    int channel_id);
  virtual supla_channel_electricity_measurement *_get_electricity_measurement(
      void);
  virtual supla_channel_ic_measurement *_get_impulse_counter_measurement(void);

 public:
  MqttStateMessageProviderMock();
  virtual ~MqttStateMessageProviderMock();
};

#endif /* MQTTSTATEMESSAGEPROVIDERMOCK_H_ */
