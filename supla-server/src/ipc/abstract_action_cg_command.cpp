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

#include "ipc/abstract_action_cg_command.h"

#include "actions/abstract_action_config.h"
#include "proto.h"

supla_abstract_action_cg_command::supla_abstract_action_cg_command(
    supla_abstract_ipc_socket_adapter *socket_adapter, int action)
    : supla_abstract_ipc_command(socket_adapter) {
  this->action = action;
}

const std::string supla_abstract_action_cg_command::get_command_name(void) {
  switch (action) {
    case ACTION_OPEN:
      return "ACTION-CG-OPEN:";
    case ACTION_CLOSE:
      return "ACTION-CG-CLOSE:";
    case ACTION_TOGGLE:
      return "ACTION-CG-TOGGLE:";
    case ACTION_STOP:
      return "ACTION-CG-STOP:";
    case ACTION_COPY:
      return "ACTION-CG-COPY:";
    case ACTION_UP_OR_STOP:
      return "ACTION-CG-UP-OR-STOP:";
    case ACTION_DOWN_OR_STOP:
      return "ACTION-CG-DOWN-OR-STOP:";
    case ACTION_STEP_BY_STEP:
      return "ACTION-CG-SBS:";
  }
  return "";
}

void supla_abstract_action_cg_command::_send_result(bool success,
                                                    int channel_id) {
  if (success) {
    send_result("OK:", channel_id);
  } else {
    send_result("FAIL:", channel_id);
  }
}

void supla_abstract_action_cg_command::on_command_match(const char *params) {
  if (!params) {
    send_result("UNKNOWN:0");
    return;
  }

  int user_id = 0;
  int group_id = 0;
  supla_user *user = NULL;

  if (action == ACTION_COPY) {
    int source_device_id = 0;
    int source_channel_id = 0;

    sscanf(params, "%i,%i,%i,%i", &user_id, &group_id, &source_device_id,
           &source_channel_id);

    if (user_id && group_id && source_device_id && source_channel_id &&
        (user = supla_user::find(user_id, false)) != NULL) {
      bool result =
          action_copy(user, group_id, source_device_id, source_channel_id);
      _send_result(result, group_id);
    } else {
      send_result("UNKNOWN:", group_id);
    }

    return;
  }

  if (params) {
    sscanf(params, "%i,%i", &user_id, &group_id);

    if (user_id && group_id &&
        (user = supla_user::find(user_id, false)) != NULL) {
      bool result = false;

      switch (action) {
        case ACTION_OPEN:
        case ACTION_CLOSE:
          result = action_open_close(user, group_id, action == ACTION_OPEN);
          break;
        case ACTION_TOGGLE:
          result = action_toggle(user, group_id);
          break;
        case ACTION_STOP:
          result = action_stop(user, group_id);
          break;
        case ACTION_UP_OR_STOP:
          result = action_up_or_stop(user, group_id);
          break;
        case ACTION_DOWN_OR_STOP:
          result = action_down_or_stop(user, group_id);
          break;
        case ACTION_STEP_BY_STEP:
          result = action_step_by_step(user, group_id);
          break;
      }

      _send_result(result, group_id);
      return;
    }
  }
  send_result("UNKNOWN:", group_id);
}
