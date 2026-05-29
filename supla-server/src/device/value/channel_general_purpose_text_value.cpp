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

#include "channel_general_purpose_text_value.h"

#include <string.h>

#include "proto.h"

supla_channel_general_purpose_text_value::
    supla_channel_general_purpose_text_value()
    : supla_abstract_channel_value() {}

supla_channel_general_purpose_text_value::
    supla_channel_general_purpose_text_value(
        const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {}

supla_abstract_channel_value *supla_channel_general_purpose_text_value::copy(
    void) const {  // NOLINT
  return new supla_channel_general_purpose_text_value(raw_value);
}

bool supla_channel_general_purpose_text_value::get_vbt_text_value(
    _vbt_var_name_e var_name, std::string *value) {
  if (var_name != var_name_none || !value) {
    return false;
  }

  *value = std::string(raw_value, strnlen(raw_value, SUPLA_CHANNELVALUE_SIZE));
  return true;
}

nlohmann::json supla_channel_general_purpose_text_value::get_template_data(
    void) {
  nlohmann::json result = supla_vbt_value::get_template_data();
  std::string value(raw_value, strnlen(raw_value, SUPLA_CHANNELVALUE_SIZE));
  result["value"] = value;
  return result;
}

// static
bool supla_channel_general_purpose_text_value::is_function_supported(
    int func) {
  return func == SUPLA_CHANNELFNC_GENERAL_PURPOSE_TEXT;
}
