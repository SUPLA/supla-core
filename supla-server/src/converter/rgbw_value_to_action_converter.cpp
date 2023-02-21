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
#include "converter/rgbw_value_to_action_converter.h"

#include "device/value/channel_rgbw_value.h"
#include "proto.h"

bool rgbw_value_to_action_converter::convert(
    supla_channel_value *value,
    supla_abstract_action_executor *action_executor) {
  supla_channel_rgbw_value *rgbw =
      dynamic_cast<supla_channel_rgbw_value *>(value);

  if (rgbw && action_executor) {
    char on_off = 0;

    unsigned int color = rgbw->get_color();
    char brightness = rgbw->get_brightness();
    char color_brightness = rgbw->get_color_brightness();

    if (brightness == 0) {
      on_off = RGBW_BRIGHTNESS_ONOFF;
    }

    if (color_brightness == 0) {
      on_off |= RGBW_COLOR_ONOFF;
    }

    action_executor->set_rgbw(&color, &color_brightness, &brightness, &on_off);
    return true;
  }

  return false;
}
