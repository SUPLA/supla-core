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

#include "on_channel_config_changed_command.h"

#include <memory>

#include "amazon/alexa_delete_request.h"
#include "device/device_dao.h"
#include "http/http_event_hub.h"
#include "mqtt/mqtt_client_suite.h"
#include "user.h"

using std::shared_ptr;

supla_on_channel_config_changed_command::
    supla_on_channel_config_changed_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_on_channel_config_changed_command(socket_adapter) {}

void supla_on_channel_config_changed_command::on_channel_config_changed(
    int user_id, int device_id, int channel_id, int type, int func,
    unsigned long long scope) {
  supla_user *user = supla_user::find(user_id, false);
  if (!user) {
    return;
  }

  if (scope & CONFIG_CHANGE_SCOPE_ALEXA_INTEGRATION_ENABLED) {
    supla_alexa_delete_request::new_request(user, channel_id, func);
  }

  supla_http_event_hub::on_voice_assistant_sync_needed(user, get_caller());

  supla_mqtt_client_suite::globalInstance()->onDeviceSettingsChanged(user_id,
                                                                     device_id);

  switch (type) {
    case SUPLA_CHANNELTYPE_HVAC:
    case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT:
    case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_METER:
      break;
    case SUPLA_CHANNELTYPE_IMPULSE_COUNTER:
      scope =
          scope &
          CONFIG_CHANGE_SCOPE_OCR;  // In the case of a pulse counter with OCR,
                                    // we only react to changes in the OCR area.
      break;
    default:
      return;
  }

  supla_db_access_provider dba;
  supla_device_dao dao(&dba);

  if (scope & CONFIG_CHANGE_SCOPE_FUNCTION) {
    dao.erase_channel_properties(user_id, channel_id);
  }

  supla_json_config *json_config =
      dao.get_channel_config(channel_id, nullptr, nullptr);

  shared_ptr<supla_device> device = user->get_devices()->get(device_id);

  if (device) {
    if (scope & CONFIG_CHANGE_SCOPE_FUNCTION) {
      device->reconnect();
    } else if (json_config &&
               ((scope & CONFIG_CHANGE_SCOPE_JSON_DEFAULT) ||
                (scope & CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE) ||
                (scope & CONFIG_CHANGE_SCOPE_JSON_ALT_WEEKLY_SCHEDULE) ||
                (scope & CONFIG_CHANGE_SCOPE_OCR))) {
      device->get_channels()->access_channel(
          channel_id, [&](supla_device_channel *channel) -> void {
            channel->set_json_config(new supla_json_config(json_config, true));

            if (scope & CONFIG_CHANGE_SCOPE_JSON_DEFAULT) {
              channel->send_config_to_device(SUPLA_CONFIG_TYPE_DEFAULT);
            }

            if (scope & CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE) {
              channel->send_config_to_device(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE);
            }

            if (scope & CONFIG_CHANGE_SCOPE_JSON_ALT_WEEKLY_SCHEDULE) {
              channel->send_config_to_device(
                  SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE);
            }

            if (scope & CONFIG_CHANGE_SCOPE_OCR) {
              channel->send_config_to_device(SUPLA_CONFIG_TYPE_OCR);
            }
          });
    }
  }

  if ((scope & CONFIG_CHANGE_SCOPE_FUNCTION) ||
      (scope & CONFIG_CHANGE_SCOPE_CAPTION) ||
      (scope & CONFIG_CHANGE_SCOPE_LOCATION) ||
      (scope & CONFIG_CHANGE_SCOPE_VISIBILITY) ||
      (scope & CONFIG_CHANGE_SCOPE_RELATIONS) ||
      (scope & CONFIG_CHANGE_SCOPE_ICON)) {
    user->reconnect(supla_caller(ctIPC), false, true);
  } else {
    if (scope & CONFIG_CHANGE_SCOPE_JSON_DEFAULT) {
      user->get_clients()->update_json_config(
          channel_id, SUPLA_CONFIG_TYPE_DEFAULT, json_config);
    }

    if ((scope & CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE) ||
        (scope & CONFIG_CHANGE_SCOPE_JSON_ALT_WEEKLY_SCHEDULE)) {
      user->get_clients()->update_json_config(
          channel_id, SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE, json_config);
    }
  }

  if (json_config) {
    delete json_config;
  }
}
