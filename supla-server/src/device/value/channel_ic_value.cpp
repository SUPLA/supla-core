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

#include "channel_ic_value.h"

#include <string.h>

#include "channeljsonconfig/impulse_counter_config.h"

supla_channel_ic_value::supla_channel_ic_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

supla_channel_ic_value::supla_channel_ic_value(
    const TDS_ImpulseCounter_Value *value)
    : supla_channel_value() {
  memcpy(raw_value, value, sizeof(TDS_ImpulseCounter_Value));
}

const TDS_ImpulseCounter_Value *supla_channel_ic_value::get_ic_value(void) {
  return (TDS_ImpulseCounter_Value *)raw_value;
}

void supla_channel_ic_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, channel_json_config *json_config,
    _logger_purpose_t *logger_data) {
  impulse_counter_config config(json_config);

  if (logger_data && !config.should_be_added_to_history()) {
    get_raw_value(logger_data->value);
  }

  config.add_initial_value(
      &reinterpret_cast<TDS_ImpulseCounter_Value *>(raw_value)->counter);

  if (logger_data && config.should_be_added_to_history()) {
    get_raw_value(logger_data->value);
  }
}
