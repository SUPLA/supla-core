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

#include "ipc/get_action_trigger_value_command.h"

#include <cstdio>
#include <memory>
#include <string>

#include "device.h"
#include "user.h"

using std::shared_ptr;
using std::string;

supla_get_action_trigger_value_command::supla_get_action_trigger_value_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const string supla_get_action_trigger_value_command::get_command_name(void) {
  return "GET-ACTION-TRIGGER-VALUE:";
}

void supla_get_action_trigger_value_command::on_command_match(
    const char *params) {
  process_parameters(
      params, [this](int user_id, int device_id, int channel_id) -> bool {
        TActionTriggerProperties value = {};

        if (get_channel_action_trigger_value(user_id, device_id, channel_id,
                                             &value)) {
          char buffer[100] = {};
          snprintf(buffer, sizeof(buffer), "VALUE:%u,%u",
                   static_cast<unsigned int>(value.ButtonMode),
                   static_cast<unsigned int>(value.Flags));
          send_result(buffer);
          return true;
        }

        return false;
      });
}

bool supla_get_action_trigger_value_command::get_channel_action_trigger_value(
    int user_id, int device_id, int channel_id,
    TActionTriggerProperties *value) {
  shared_ptr<supla_device> device =
      supla_user::get_device(user_id, device_id, channel_id);
  if (device != nullptr) {
    return device->get_channels()->get_action_trigger_value(channel_id, value);
  }
  return false;
}
