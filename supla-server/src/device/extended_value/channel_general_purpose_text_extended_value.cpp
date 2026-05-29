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

#include "channel_general_purpose_text_extended_value.h"

#include <string.h>

#include <string>

#include "proto.h"

using std::string;

supla_channel_general_purpose_text_extended_value::
    supla_channel_general_purpose_text_extended_value()
    : supla_abstract_channel_extended_value() {}

supla_channel_general_purpose_text_extended_value::
    supla_channel_general_purpose_text_extended_value(
        const TSuplaChannelExtendedValue *value)
    : supla_abstract_channel_extended_value(value) {}

supla_channel_general_purpose_text_extended_value::
    ~supla_channel_general_purpose_text_extended_value(void) {}

supla_abstract_channel_extended_value *
supla_channel_general_purpose_text_extended_value::copy(void) {  // NOLINT
  supla_channel_general_purpose_text_extended_value *result =
      new supla_channel_general_purpose_text_extended_value();
  result->set_raw_value(get_value_ptr());
  return result;
}

bool supla_channel_general_purpose_text_extended_value::get_vbt_text_value(
    _vbt_var_name_e var_name, std::string *value) {
  if (var_name != var_name_none || !value) {
    return false;
  }

  *value = get_text();
  return true;
}

nlohmann::json supla_channel_general_purpose_text_extended_value::
get_template_data(void) {
  nlohmann::json result = supla_abstract_channel_extended_value::
      get_template_data();
  result["value"] = get_text();
  return result;
}

string supla_channel_general_purpose_text_extended_value::get_text(
    void) {
  TSuplaChannelExtendedValue ev = {};
  if (!get_raw_value(&ev) || ev.size == 0) {
    return "";
  }

  // Ensure null-termination within the allowed size
  unsigned int len = ev.size < SUPLA_GENERAL_PURPOSE_TEXT_MAX_SIZE
                         ? ev.size
                         : SUPLA_GENERAL_PURPOSE_TEXT_MAX_SIZE - 1;
  ev.value[len] = '\0';
  return string(ev.value, strnlen(ev.value, len + 1));
}

// static
bool supla_channel_general_purpose_text_extended_value::is_ev_type_supported(
    char type) {
  return type == EV_TYPE_GENERAL_PURPOSE_TEXT;
}
