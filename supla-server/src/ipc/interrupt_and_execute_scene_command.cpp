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

#include "ipc/interrupt_and_execute_scene_command.h"

#include "http/httprequestqueue.h"
#include "scene/scene_asynctask.h"

supla_interrupt_and_execute_scene_command::
    supla_interrupt_and_execute_scene_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_interrupt_and_execute_scene_command(socket_adapter) {}

bool supla_interrupt_and_execute_scene_command::interrupt_and_execute(
    int user_id, int scene_id, const char *google_request_id) {
  bool result =
      supla_scene_asynctask::execute(
          supla_scene_asynctask::get_queue(), supla_scene_asynctask::get_pool(),
          get_caller(), user_id, scene_id, true) == serOK;

  if (google_request_id) {
    // For Google Home always confirm that the scene is being executed
    supla_http_request_queue::getInstance()->onSceneExecuted(
        get_caller(), user_id, scene_id, google_request_id);
  }

  return result;
}
