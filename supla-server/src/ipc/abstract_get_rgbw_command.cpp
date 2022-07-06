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

#include "ipc/abstract_get_rgbw_command.h"

supla_abstract_get_rgbw_command::supla_abstract_get_rgbw_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const std::string supla_abstract_get_rgbw_command::get_command_name(void) {
  return "GET-RGBW-VALUE:";
}

void supla_abstract_get_rgbw_command::on_command_match(const char *params) {
  process_parameters(
      params, [this](int user_id, int device_id, int channel_id) -> bool {
        int color = 0;
        char color_brightness = 0;
        char brightness = 0;
        char on_off = 0;

        if (get_channel_rgbw_value(user_id, device_id, channel_id, &color,
                                   &color_brightness, &brightness, &on_off)) {
          char result[100] = {};
          snprintf(result, sizeof(result), "VALUE:%i,%i,%i", color,
                   color_brightness, brightness);

          send_result(result);
          return true;
        }

        return false;
      });
}
