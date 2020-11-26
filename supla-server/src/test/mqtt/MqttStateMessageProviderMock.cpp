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
    : supla_mqtt_state_message_provider_abstract() {
  cvalue_mock = NULL;
}

MqttStateMessageProviderMock::~MqttStateMessageProviderMock(void) {}

channel_complex_value *MqttStateMessageProviderMock::_get_complex_value(
    int user_id, int device_id, int channel_id) {
  channel_complex_value *cvalue =
      (channel_complex_value *)malloc(sizeof(channel_complex_value));
  if (cvalue_mock) {
    memcpy(cvalue, cvalue_mock, sizeof(channel_complex_value));
  } else {
    memset(cvalue, 0, sizeof(channel_complex_value));
  }

  return cvalue;
}

supla_channel_electricity_measurement *
MqttStateMessageProviderMock::_get_electricity_measurement(void) {
  return NULL;
}

supla_channel_ic_measurement *
MqttStateMessageProviderMock::_get_impulse_counter_measurement(void) {
  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1230;

  char customUnit[] = "m3";
  char currency[] = "EUR";

  return new supla_channel_ic_measurement(123, SUPLA_CHANNELFNC_IC_GAS_METER,
                                          &ic_val, currency, customUnit, 10000,
                                          1000);
}

void MqttStateMessageProviderMock::setComplexValue(
    channel_complex_value *cvalue) {
  cvalue_mock = cvalue;
}
