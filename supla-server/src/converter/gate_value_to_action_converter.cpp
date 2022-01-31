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
#include "converter/gate_value_to_action_converter.h"

#include "device/channel_gate_value.h"

bool gate_value_to_action_converter::convert(
    supla_channel_value *value,
    supla_abstract_action_executor *action_executor) {
  supla_channel_gate_value *gval =
      dynamic_cast<supla_channel_gate_value *>(value);

  if (gval && action_executor) {
    _gate_sensor_level_enum opening_sensor_level =
        gval->get_opening_sensor_level();
    if (opening_sensor_level == gsl_open) {
      action_executor->open();
    } else if (opening_sensor_level == gsl_closed) {
      action_executor->close();
    }
    return opening_sensor_level != gsl_unknown;
  }
  return false;
}
