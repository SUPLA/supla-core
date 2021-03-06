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
    : supla_mqtt_abstract_state_message_provider() {
  cvalue_mock = NULL;
  m_count = 1;
  measured_values = EM_VAR_ALL;
  snprintf(suid, SHORT_UNIQUEID_MAXSIZE, "9920767494dd87196e1896c7cbab707c");
}

MqttStateMessageProviderMock::~MqttStateMessageProviderMock(void) {}

const char *MqttStateMessageProviderMock::_get_user_suid(void) { return suid; }

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
  TElectricityMeter_ExtendedValue_V2 em_ev;
  memset(&em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V2));

  em_ev.m[0].freq = 4997;

  for (short a = 0; a < 3; a++) {
    em_ev.m[0].voltage[a] = 23706 + a * 10000;
    em_ev.m[0].current[a] = 1123 + a * 1000;
    em_ev.m[0].power_active[a] = 111223 + a * 100000;
    em_ev.m[0].power_reactive[a] = 211223 + a * 100000;
    em_ev.m[0].power_apparent[a] = 311223 + a * 100000;
    em_ev.m[0].power_factor[a] = 1331 + a * 1000;
    em_ev.m[0].phase_angle[a] = 11 + a * 10;

    em_ev.total_forward_active_energy[a] = 1230000 + a * 1000000;
    em_ev.total_reverse_active_energy[a] = 2230000 + a * 1000000;
    em_ev.total_forward_reactive_energy[a] = 3230000 + a * 1000000;
    em_ev.total_reverse_reactive_energy[a] = 4230000 + a * 1000000;

    em_ev.total_forward_active_energy_balanced +=
        em_ev.total_forward_active_energy[a] + 100000;
    em_ev.total_reverse_active_energy_balanced +=
        em_ev.total_reverse_active_energy[a] + 100000;
  }

  em_ev.measured_values = measured_values;
  em_ev.period = 1;
  em_ev.m_count = m_count;

  char currency[] = "EUR";
  return new supla_channel_electricity_measurement(123, &em_ev, 10000,
                                                   currency);
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

void MqttStateMessageProviderMock::setMeasuredValues(
    _supla_int_t measured_values) {
  this->measured_values = measured_values;
}

void MqttStateMessageProviderMock::setMeasurementCount(_supla_int_t m_count) {
  this->m_count = m_count;
}
