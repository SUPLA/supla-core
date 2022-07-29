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

#include "ipc/abstract_set_cg_rgbw_command.h"

#include "ipc/abstract_set_rgbw_command.h"

using std::string;

supla_abstract_set_cg_rgbw_command::supla_abstract_set_cg_rgbw_command(
    supla_abstract_ipc_socket_adapter *socket_adapter, bool random_color)
    : supla_abstract_ipc_command(socket_adapter) {
  this->random_color = random_color;
}

const string supla_abstract_set_cg_rgbw_command::get_command_name(void) {
  return random_color ? "SET-CG-RAND-RGBW-VALUE:" : "SET-CG-RGBW-VALUE:";
}

void supla_abstract_set_cg_rgbw_command::on_command_match(const char *params) {
  int user_id = 0;
  int group_id = 0;
  int color = 0;
  int color_brightness = 0;
  int brightness = 0;
  int turn_onoff = 0;

  if (params) {
    if (random_color) {
      sscanf(params, "%i,%i,%i,%i,%i", &user_id, &group_id, &color_brightness,
             &brightness, &turn_onoff);
      color = supla_abstract_set_rgbw_command::get_random_color();
    } else {
      sscanf(params, "%i,%i,%i,%i,%i,%i", &user_id, &group_id, &color,
             &color_brightness, &brightness, &turn_onoff);
    }

    supla_user *user = NULL;

    if (user_id && group_id &&
        (user = supla_user::find(user_id, false)) != NULL) {
      if (color_brightness < 0 || color_brightness > 100 || brightness < 0 ||
          brightness > 100) {
        send_result("VALUE OUT OF RANGE");
        return;
      }

      if (set_cg_rgbw_value(user, group_id, color, color_brightness, brightness,
                            turn_onoff)) {
        send_result("OK:", group_id);
      } else {
        send_result("FAIL:", group_id);
      }
      return;
    }
  }
  send_result("UNKNOWN:", group_id);
}
