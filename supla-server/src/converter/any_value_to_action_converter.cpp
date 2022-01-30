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

#include "converter/any_value_to_action_converter.h"

#include "converter/rs_value_to_action_converter.h"

any_value_to_action_converter::any_value_to_action_converter() {}

any_value_to_action_converter::~any_value_to_action_converter() {}

bool any_value_to_action_converter::convert(
    const char value[SUPLA_CHANNELVALUE_SIZE], int channel_func,
    supla_abstract_action_executor *action_executor) {
  if (!channel_func || !action_executor) {
    return false;
  }

  bool result = false;

  abstract_value_to_action_converter *converter = NULL;

  switch (channel_func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      converter = new rs_value_to_action_converter();
      break;
  }

  if (converter) {
    result = converter->convert(value, action_executor);
    delete converter;
    converter = NULL;
  }

  return result;
}
