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

#include <ipc/abstract_on_scene_changed_command.h>

supla_abstract_on_scene_changed_command::
    supla_abstract_on_scene_changed_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const std::string supla_abstract_on_scene_changed_command::get_command_name(
    void) {
  return "USER-ON-SCENE-CHANGED:";
}

void supla_abstract_on_scene_changed_command::on_command_match(
    const char *params) {
  int user_id = 0;
  int scene_id = 0;

  if (params) {
    sscanf(params, "%i,%i", &user_id, &scene_id);

    if (user_id && scene_id) {
      on_scene_changed(user_id, scene_id);
      send_result("OK:", scene_id);
      return;
    }
  }
  send_result("UNKNOWN:", scene_id);
}
