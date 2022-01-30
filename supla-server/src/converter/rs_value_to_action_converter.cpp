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

bool rs_value_to_action_converter::convert(
    const char value[SUPLA_CHANNELVALUE_SIZE],
    supla_abstract_action_executor *action_executor) {
  if (value && action_executor) {
    TDSC_RollerShutterValue *rs_val = (TDSC_RollerShutterValue *)value;
    if (rs_val->position >= 0 && rs_val->position <= 100) {
      action_executor->shut((const char *)&rs_val->position);
    }
  }
  return false;
}
