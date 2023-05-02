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

#include "vbt_binary_condition.h"

#include <string.h>

#include "device/value/channel_binary_sensor_value.h"

supla_vbt_binary_condition::supla_vbt_binary_condition(void)
    : supla_abstract_vbt_bool_condition() {}

supla_vbt_binary_condition::supla_vbt_binary_condition(cJSON *json)
    : supla_abstract_vbt_bool_condition(json) {
  cJSON *on_change_to = cJSON_GetObjectItem(json, "on_change_to");
  if (on_change_to && cJSON_IsString(on_change_to)) {
    if (strncmp(cJSON_GetStringValue(on_change_to), "hi", 10) == 0) {
      set_expected(vbt_bool_true);
    } else if (strncmp(cJSON_GetStringValue(on_change_to), "low", 10) == 0) {
      set_expected(vbt_bool_false);
    }
  }
}

supla_vbt_binary_condition::~supla_vbt_binary_condition(void) {}

bool supla_vbt_binary_condition::convert(supla_channel_value *value,
                                         _vbt_bool_e *converted) {
  if (dynamic_cast<supla_channel_binary_sensor_value *>(value)) {
    *converted =
        dynamic_cast<supla_channel_binary_sensor_value *>(value)->is_hi()
            ? vbt_bool_true
            : vbt_bool_false;
  }

  return false;
}

bool supla_vbt_binary_condition::equal(supla_abstract_vbt_condition *cnd) {
  return dynamic_cast<supla_vbt_binary_condition *>(cnd) &&
         dynamic_cast<supla_vbt_binary_condition *>(cnd)->get_expected() ==
             get_expected();
}

supla_abstract_vbt_condition *supla_vbt_binary_condition::copy(void) {
  supla_vbt_binary_condition *cnd = new supla_vbt_binary_condition();
  cnd->set_expected(get_expected());
  return cnd;
}

// static
bool supla_vbt_binary_condition::is_function_supported(int func) {
  return supla_channel_binary_sensor_value::is_function_supported(func);
}
