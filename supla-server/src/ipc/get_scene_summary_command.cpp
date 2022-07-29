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

#include "ipc/get_scene_summary_command.h"

#include "scene/scene_asynctask.h"

supla_get_scene_summary_command::supla_get_scene_summary_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_get_scene_summary_command(socket_adapter) {}

bool supla_get_scene_summary_command::get_scene_state(
    int user_id, int scene_id, supla_scene_state *state) {
  return supla_scene_asynctask::get_scene_state(
      supla_scene_asynctask::get_queue(), user_id, scene_id, state);
}
