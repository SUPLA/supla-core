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

#include "channel_general_purpose_base_value.h"

#include <math.h>

#include "jsonconfig/channel/general_purpose_base_config.h"

using std::string;

supla_channel_general_purpose_base_value::
    supla_channel_general_purpose_base_value()
    : supla_channel_value() {}

supla_channel_general_purpose_base_value::
    supla_channel_general_purpose_base_value(
        const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

double supla_channel_general_purpose_base_value::get_value(void) {
  return *(double *)raw_value;
}

void supla_channel_general_purpose_base_value::set_value(double value) {
  *(double *)raw_value = value;
}

void supla_channel_general_purpose_base_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, supla_json_config *json_config) {
  general_purpose_base_config config(json_config);

  double value = get_value();

  if (value != NAN) {
    double divider = config.get_value_divider();
    double multiplier = config.get_value_multiplier();

    if (divider) {
      value /= divider;
    }

    if (multiplier) {
      value *= multiplier;
    }

    set_value(value + config.get_value_added());
  }
}

string supla_channel_general_purpose_base_value::value_to_str(double value,
                                                              string *nan) {
  if (value == NAN) {
    return nan ? *nan : "nan";
  } else {
    return std::to_string(value);
  }
}

string supla_channel_general_purpose_base_value::get_value_str() {
  return value_to_str(get_value(), nullptr);
}

string supla_channel_general_purpose_base_value::get_value_str(string nan) {
  return value_to_str(get_value(), &nan);
}
