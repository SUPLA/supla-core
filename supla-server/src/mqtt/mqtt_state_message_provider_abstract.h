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

#ifndef MQTT_STATE_MESSAGE_PROVIDER_ABSTRACT_H_
#define MQTT_STATE_MESSAGE_PROVIDER_ABSTRACT_H_

#include "devicechannel.h"
#include "mqtt_db.h"
#include "mqtt_message_provider.h"

class supla_mqtt_state_message_provider_abstract
    : public supla_mqtt_message_provider {
 private:
  int user_id;
  int device_id;
  int channel_id;

  channel_complex_value *cvalue;
  supla_channel_electricity_measurement *em;
  supla_channel_ic_measurement *icm;

 protected:
  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);

  virtual channel_complex_value *_get_complex_value(int user_id, int device_id,
                                                    int channel_id) = 0;
  virtual supla_channel_electricity_measurement *_get_electricity_measurement(
      void) = 0;
  virtual supla_channel_ic_measurement *_get_impulse_counter_measurement(
      void) = 0;

  channel_complex_value *get_complex_value(void);
  supla_channel_electricity_measurement *get_electricity_measurement(void);
  supla_channel_ic_measurement *get_impulse_counter_measurement(void);

  bool get_message_at_index(unsigned short index, const char *topic_prefix,
                            char **topic_name, void **message,
                            size_t *message_size);

 public:
  supla_mqtt_state_message_provider_abstract(void);
  virtual ~supla_mqtt_state_message_provider_abstract(void);
  void set_data(int user_id, int device_id, int channel_id);
};

#endif /*MQTT_STATE_MESSAGE_PROVIDER_ABSTRACT_H_*/
