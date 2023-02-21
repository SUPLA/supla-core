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
#include "converter/rs_value_to_action_converter.h"

#include "device/value/channel_rs_value.h"

bool rs_value_to_action_converter::convert(
    supla_channel_value *value,
    supla_abstract_action_executor *action_executor) {
  supla_channel_rs_value *rs_value =
      dynamic_cast<supla_channel_rs_value *>(value);

  if (rs_value && action_executor) {
    char position = rs_value->get_rs_value()->position;
    if (position >= 0 && position <= 100) {
      action_executor->shut(&position, false);
    }
  }
  return false;
}
