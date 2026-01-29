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

#include "ipc/set_rgbw_command.h"

#include <grp.h>

#include <cstdio>
#include <memory>
#include <string>

#include "actions/action_rgbw_parameters.h"
#include "caller.h"
#include "device.h"
#include "log.h"
#include "proto.h"
#include "user.h"

using std::shared_ptr;
using std::string;

supla_set_rgbw_command::supla_set_rgbw_command(
    supla_abstract_ipc_socket_adapter *socket_adapter,
    supla_abstract_action_executor *action_executor, bool color_random,
    bool group)
    : supla_abstract_ipc_command(socket_adapter) {
  this->color_random = color_random;
  this->group = group;
  this->action_executor = action_executor;
}

const string supla_set_rgbw_command::get_command_name(void) {
  string name = "SET-";

  if (group) {
    name.append("CG-");
  }

  if (color_random) {
    name.append("RAND-");
  }

  name.append("RGBW-VALUE:");

  return name;
}

void supla_set_rgbw_command::on_command_match(const char *params) {
  int user_id = 0;
  int device_id = 0;  // DEPRECATED
  int channel_or_group_id = 0;
  int color = 0;
  int color_brightness = 0;
  int brightness = 0;
  int turn_onoff = 0;
  int command = 0;
  int white_temperature = 0;

  int ret = 0;
  int expected = 8;

  if (params) {
    if (group) {
      if (color_random) {
        ret = sscanf(params, "%i,%i,%i,%i,%i,%i,%i", &user_id,
                     &channel_or_group_id, &color_brightness, &brightness,
                     &turn_onoff, &command, &white_temperature);
        expected--;
      } else {
        ret = sscanf(params, "%i,%i,%i,%i,%i,%i,%i,%i", &user_id,
                     &channel_or_group_id, &color, &color_brightness,
                     &brightness, &turn_onoff, &command, &white_temperature);
      }
    } else {
      if (color_random) {
        ret = sscanf(params, "%i,%i,%i,%i,%i,%i,%i,%i", &user_id, &device_id,
                     &channel_or_group_id, &color_brightness, &brightness,
                     &turn_onoff, &command, &white_temperature);
      } else {
        ret = sscanf(params, "%i,%i,%i,%i,%i,%i,%i,%i,%i", &user_id, &device_id,
                     &channel_or_group_id, &color, &color_brightness,
                     &brightness, &turn_onoff, &command, &white_temperature);
        expected++;
      }
    }

    if (ret != expected) {
      send_result("INCORRECT NUMBERS OF PARAMETERS");
      return;
    }

    if (user_id && channel_or_group_id) {
      supla_action_rgbw_parameters params;
      if (!params.set_params(color, color_random, color_brightness, brightness,
                             turn_onoff, command, white_temperature)) {
        send_result("VALUE OUT OF RANGE");
        return;
      }

      action_executor->execute_action(
          supla_caller(), user_id, ACTION_SET_RGBW_PARAMETERS,
          group ? stChannelGroup : stChannel, channel_or_group_id, nullptr,
          &params, 0, 0, 0, nullptr, get_alexa_correlation_token(),
          get_google_request_id());
      send_result("OK:", channel_or_group_id);
      return;
    }
  }
  send_result("UNKNOWN:", channel_or_group_id);
}
