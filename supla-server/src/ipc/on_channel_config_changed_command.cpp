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
#include "suplan/server_peer_transport.h"
#include "user.h"

using std::shared_ptr;

supla_on_channel_config_changed_command::
    supla_on_channel_config_changed_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_on_channel_config_changed_command(socket_adapter) {}

void supla_on_channel_config_changed_command::on_channel_config_changed(
    int user_id, int device_id, int channel_id, int type, int func,
    unsigned long long scope) {
  // Notifications carry no authorization. Even a cold/offline user must have
  // the current committed business projection reconciled before delivery.
  supla_mariadb_access_provider dba;
  supla_device_dao dao(&dba);
  std::unique_ptr<supla_json_config> authoritative;
  bool reconciled = true;
  if ((scope &
       (CONFIG_CHANGE_SCOPE_RELATIONS | CONFIG_CHANGE_SCOPE_FUNCTION)) &&
      !supla_suplan_server_peers::reconcile_manual(user_id, channel_id))
    reconciled = false;
  if (type == SUPLA_CHANNELTYPE_HVAC || type == SUPLA_CHANNELTYPE_CONTAINER ||
      (type == SUPLA_CHANNELTYPE_VALVE_OPENCLOSE ||
       type == SUPLA_CHANNELTYPE_VALVE_PERCENTAGE) ||
      (scope & CONFIG_CHANGE_SCOPE_FUNCTION)) {
    if (!supla_suplan_server_peers::reconcile_hvac(user_id, channel_id,
                                                   &authoritative))
      reconciled = false;
  }
  if (((scope & CONFIG_CHANGE_SCOPE_FUNCTION) ||
       (type == SUPLA_CHANNELTYPE_HVAC &&
        (scope & CONFIG_CHANGE_SCOPE_JSON_DEFAULT))) &&
      !supla_suplan_server_peers::reconcile_dependencies(user_id, channel_id)) {
    reconciled = false;
  }
  if (!reconciled) return;
  supla_user *user = supla_user::find(user_id, false);
  if (!user) {
    return;
  }

  if (!user->get_devices()->get(device_id)) {
    user->get_devices()->on_channel_config_changed(device_id, channel_id);
  }

  if (scope & CONFIG_CHANGE_SCOPE_ALEXA_INTEGRATION_ENABLED) {
    supla_alexa_delete_request::new_request(user, channel_id, func);
  }

  supla_http_event_hub::on_voice_assistant_sync_needed(user, get_caller());

  supla_mqtt_client_suite::globalInstance()->onDeviceSettingsChanged(user_id,
                                                                     device_id);

  bool force_device_reconnect = false;

  switch (type) {
    case SUPLA_CHANNELTYPE_HVAC:
    case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT:
    case SUPLA_CHANNELTYPE_GENERAL_PURPOSE_METER:
    case SUPLA_CHANNELTYPE_CONTAINER:
    case SUPLA_CHANNELTYPE_VALVE_OPENCLOSE:
    case SUPLA_CHANNELTYPE_VALVE_PERCENTAGE:
      break;
    case SUPLA_CHANNELTYPE_ELECTRICITY_METER:
    case SUPLA_CHANNELTYPE_SENSORNO:
    case SUPLA_CHANNELTYPE_THERMOMETER:
    case SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR:
    case SUPLA_CHANNELTYPE_HUMIDITYSENSOR:
      force_device_reconnect = true;
      break;
    case SUPLA_CHANNELTYPE_IMPULSE_COUNTER:
      scope =
          scope &
          CONFIG_CHANGE_SCOPE_OCR;  // In the case of a pulse counter with OCR,
                                    // we only react to changes in the OCR area.
      break;
    default:
      switch (func) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
        case SUPLA_CHANNELFNC_STAIRCASETIMER:
          if (!(scope & CONFIG_CHANGE_SCOPE_FUNCTION)) {
            return;
          }
          break;
        default:
          return;
      }
  }

  if (scope & CONFIG_CHANGE_SCOPE_FUNCTION) {
    dao.erase_channel_properties(user_id, channel_id);
  }

  supla_json_config *json_config =
      authoritative ? authoritative.release()
                    : dao.get_channel_config(channel_id, nullptr, nullptr);

  shared_ptr<supla_device> device = user->get_devices()->get(device_id);

  if (device) {
    if ((scope & CONFIG_CHANGE_SCOPE_FUNCTION) || force_device_reconnect) {
      device->reconnect();
    } else if (json_config &&
               ((scope & CONFIG_CHANGE_SCOPE_JSON_DEFAULT) ||
                (scope & CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE) ||
                (scope & CONFIG_CHANGE_SCOPE_JSON_ALT_WEEKLY_SCHEDULE) ||
                (scope & CONFIG_CHANGE_SCOPE_OCR))) {
      device->get_channels()->access_channel(
          channel_id, [&](supla_device_channel *channel) -> void {
            auto publish = [&](unsigned char type) {
              channel->request_config_publication(type);
            };

            if (scope & CONFIG_CHANGE_SCOPE_JSON_DEFAULT) {
              publish(SUPLA_CONFIG_TYPE_DEFAULT);
              if (channel->get_func() == SUPLA_CHANNELFNC_STAIRCASETIMER) {
                publish(SUPLA_CONFIG_TYPE_EXTENDED);
              }
            }

            if (scope & CONFIG_CHANGE_SCOPE_JSON_WEEKLY_SCHEDULE) {
              publish(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE);
            }

            if (scope & CONFIG_CHANGE_SCOPE_JSON_ALT_WEEKLY_SCHEDULE) {
              publish(SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE);
            }

            if (scope & CONFIG_CHANGE_SCOPE_OCR) {
              publish(SUPLA_CONFIG_TYPE_OCR);
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
    user->reconnect(get_caller(), false, true);
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
