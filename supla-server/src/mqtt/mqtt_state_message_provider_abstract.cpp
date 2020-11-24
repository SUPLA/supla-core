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

#include <mqtt_state_message_provider_abstract.h>
#include <stdlib.h>
#include <string.h>

supla_mqtt_state_message_provider_abstract::
    supla_mqtt_state_message_provider_abstract(void)
    : supla_mqtt_message_provider() {
  this->user_id = 0;
  this->device_id = 0;
  this->channel_id = 0;
  this->cvalue = NULL;
  this->em = NULL;
  this->icm = NULL;
}

supla_mqtt_state_message_provider_abstract::
    ~supla_mqtt_state_message_provider_abstract(void) {
  if (cvalue) {
    free(cvalue);
  }

  if (em) {
    delete em;
  }

  if (icm) {
    delete icm;
  }
}

int supla_mqtt_state_message_provider_abstract::get_user_id(void) {
  return user_id;
}

int supla_mqtt_state_message_provider_abstract::get_device_id(void) {
  return device_id;
}

int supla_mqtt_state_message_provider_abstract::get_channel_id(void) {
  return channel_id;
}

void supla_mqtt_state_message_provider_abstract::set_data(int user_id,
                                                          int device_id,
                                                          int channel_id) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;

  if (cvalue) {
    free(cvalue);
    cvalue = NULL;
  }

  if (em) {
    delete em;
    em = NULL;
  }

  if (icm) {
    delete icm;
    icm = NULL;
  }
}

channel_complex_value *
supla_mqtt_state_message_provider_abstract::get_complex_value(void) {
  if (cvalue == NULL) {
    cvalue = _get_complex_value(user_id, device_id, channel_id);
  }

  return cvalue;
}

supla_channel_electricity_measurement *
supla_mqtt_state_message_provider_abstract::get_electricity_measurement(void) {
  if (em == NULL) {
    em = _get_electricity_measurement();
  }

  return em;
}

supla_channel_ic_measurement *
supla_mqtt_state_message_provider_abstract::get_impulse_counter_measurement(
    void) {
  if (icm == NULL) {
    icm = _get_impulse_counter_measurement();
  }

  return icm;
}

bool supla_mqtt_state_message_provider_abstract::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  return false;
}
