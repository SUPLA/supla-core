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

#include "device/extended_value/channel_ic_extended_value.h"
#include "jsonconfig/channel/impulse_counter_config.h"

supla_channel_ic_value::supla_channel_ic_value(void) : supla_channel_value() {
  memset(original_raw_value, 0, SUPLA_CHANNELVALUE_SIZE);
}

supla_channel_ic_value::supla_channel_ic_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {
  memcpy(original_raw_value, raw_value, SUPLA_CHANNELVALUE_SIZE);
}

supla_channel_ic_value::supla_channel_ic_value(
    const TDS_ImpulseCounter_Value *value)
    : supla_channel_value() {
  memcpy(raw_value, value, sizeof(TDS_ImpulseCounter_Value));
  memcpy(original_raw_value, raw_value, SUPLA_CHANNELVALUE_SIZE);
}

const TDS_ImpulseCounter_Value *supla_channel_ic_value::get_ic_value(void) {
  return (TDS_ImpulseCounter_Value *)raw_value;
}

void supla_channel_ic_value::set_raw_value(
    char raw_value[SUPLA_CHANNELVALUE_SIZE]) {
  supla_channel_value::set_raw_value(raw_value);
  memcpy(original_raw_value, raw_value, SUPLA_CHANNELVALUE_SIZE);
}

void supla_channel_ic_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, supla_json_config *json_config) {
  impulse_counter_config config(json_config);

  config.add_initial_value(
      &reinterpret_cast<TDS_ImpulseCounter_Value *>(raw_value)->counter);
}

supla_channel_extended_value *supla_channel_ic_value::convert2extended(
    supla_json_config *json_config, int func, const char *text_param1,
    const char *text_param2, int param2, int param3,
    supla_channel_extended_value **data_logger_purpose) {
  impulse_counter_config config(json_config);

  TDS_ImpulseCounter_Value ic_value = {};
  memcpy(&ic_value, original_raw_value, sizeof(TDS_ImpulseCounter_Value));

  config.add_initial_value(&ic_value.counter);

  if (data_logger_purpose) {
    if (*data_logger_purpose) {
      delete *data_logger_purpose;
      *data_logger_purpose = nullptr;
    }

    *data_logger_purpose = new supla_channel_ic_extended_value(
        func,
        config.should_be_added_to_history()
            ? &ic_value
            : reinterpret_cast<TDS_ImpulseCounter_Value *>(original_raw_value),
        text_param1, text_param2, param2, param3);
  }

  return new supla_channel_ic_extended_value(func, &ic_value, text_param1,
                                             text_param2, param2, param3);
}

// static
bool supla_channel_ic_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
      return true;
  }

  return false;
}
