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

#include "MqttStateMessageProviderMock.h"

MqttStateMessageProviderMock::MqttStateMessageProviderMock(void)
    : supla_mqtt_state_message_provider_abstract() {}

MqttStateMessageProviderMock::~MqttStateMessageProviderMock(void) {}

channel_complex_value *MqttStateMessageProviderMock::_get_complex_value(
    int user_id, int device_id, int channel_id) {
  return NULL;
}

supla_channel_electricity_measurement *
MqttStateMessageProviderMock::_get_electricity_measurement(void) {
  return NULL;
}

supla_channel_ic_measurement *
MqttStateMessageProviderMock::_get_impulse_counter_measurement(void) {
  return NULL;
}
