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

#include "ipc/abstract_action_command.h"

#include "actions/abstract_action_config.h"

using std::string;

supla_abstract_action_command::supla_abstract_action_command(
    supla_abstract_ipc_socket_adapter *socket_adapter, int action)
    : supla_abstract_ipc_command(socket_adapter) {
  this->action = action;
}

const string supla_abstract_action_command::get_command_name(void) {
  switch (action) {
    case ACTION_OPEN:
      return "ACTION-OPEN:";
    case ACTION_CLOSE:
      return "ACTION-CLOSE:";
    case ACTION_TURN_ON:
      return "ACTION-TURN-ON:";
    case ACTION_TURN_OFF:
      return "ACTION-TURN-OFF:";
    case ACTION_TOGGLE:
      return "ACTION-TOGGLE:";
    case ACTION_STOP:
      return "ACTION-STOP:";
    case ACTION_COPY:
      return "ACTION-COPY:";
    case ACTION_UP_OR_STOP:
      return "ACTION-UP-OR-STOP:";
    case ACTION_DOWN_OR_STOP:
      return "ACTION-DOWN-OR-STOP:";
    case ACTION_STEP_BY_STEP:
      return "ACTION-SBS:";
    case ACTION_SHUT_PARTIALLY:
      return "ACTION-SHUT-PARTIALLY:";
    case ACTION_HVAC_SET_PARAMETERS:
      return "ACTION-HVAC-SET-PARAMETERS";
  }
  return "";
}

void supla_abstract_action_command::_send_result(bool success, int channel_id) {
  if (success) {
    send_result("OK:", channel_id);
  } else {
    send_result("FAIL:", channel_id);
  }
}

void supla_abstract_action_command::on_command_match(const char *params) {
  if (!params) {
    send_result("UNKNOWN:0");
    return;
  }

  if (action == ACTION_SHUT_PARTIALLY) {
    int user_id = 0;
    int device_id = 0;
    int channel_id = 0;
    int percentage = 0;
    int delta = 0;

    sscanf(params, "%i,%i,%i,%i,%i", &user_id, &device_id, &channel_id,
           &percentage, &delta);

    if (user_id && device_id && channel_id) {
      char _percentage = percentage;
      bool result =
          action_shut(user_id, device_id, channel_id, &_percentage, delta > 0);
      _send_result(result, channel_id);
    } else {
      send_result("UNKNOWN:", channel_id);
    }

    return;
  }

  if (action == ACTION_COPY) {
    int user_id = 0;
    int device_id = 0;
    int channel_id = 0;
    int source_device_id = 0;
    int source_channel_id = 0;

    sscanf(params, "%i,%i,%i,%i,%i", &user_id, &device_id, &channel_id,
           &source_device_id, &source_channel_id);

    if (user_id && device_id && channel_id && source_channel_id) {
      bool result = action_copy(user_id, device_id, channel_id,
                                source_device_id, source_channel_id);
      _send_result(result, channel_id);
    } else {
      send_result("UNKNOWN:", channel_id);
    }

    return;
  }

  if (action == ACTION_SET_HVAC_PARAMETERS) {
    int user_id = 0;
    int device_id = 0;
    int channel_id = 0;

    TAction_HVAC_Parameters raw_hvac_params = {};
    unsigned int mode = 0;
    int heat = 0;
    int cool = 0;
    unsigned int flags = 0;

    sscanf(params, "%i,%i,%i,%u,%u,%i,%i,%u", &user_id, &device_id, &channel_id,
           &raw_hvac_params.DurationSec, &mode, &heat, &cool, &flags);

    if (user_id && device_id && channel_id) {
      raw_hvac_params.Mode = mode;
      raw_hvac_params.SetpointTemperatureHeat = heat;
      raw_hvac_params.SetpointTemperatureCool = cool;
      raw_hvac_params.Flags = flags;
      supla_action_hvac_parameters hvac_params(&raw_hvac_params);
      bool result = action_set_hvac_parameters(user_id, device_id, channel_id,
                                               &hvac_params);
      _send_result(result, channel_id);
    } else {
      send_result("UNKNOWN:", channel_id);
    }

    return;
  }

  process_parameters(
      params, [this](int user_id, int device_id, int channel_id) -> bool {
        bool result = false;

        switch (action) {
          case ACTION_OPEN:
          case ACTION_CLOSE:
            result = action_open_close(
                user_id, device_id, channel_id, action == ACTION_OPEN,
                get_alexa_correlation_token(), get_google_request_id());
            break;
          case ACTION_TURN_ON:
            result = action_turn_on(user_id, device_id, channel_id);
            break;
          case ACTION_TURN_OFF:
            result = action_turn_off(user_id, device_id, channel_id);
            break;
          case ACTION_TOGGLE:
            result = action_toggle(user_id, device_id, channel_id);
            break;
          case ACTION_STOP:
            result = action_stop(user_id, device_id, channel_id);
            break;
          case ACTION_UP_OR_STOP:
            result = action_up_or_stop(user_id, device_id, channel_id);
            break;
          case ACTION_DOWN_OR_STOP:
            result = action_down_or_stop(user_id, device_id, channel_id);
            break;
          case ACTION_STEP_BY_STEP:
            result = action_step_by_step(user_id, device_id, channel_id);
            break;
        }

        _send_result(result, channel_id);

        return true;
      });
}
