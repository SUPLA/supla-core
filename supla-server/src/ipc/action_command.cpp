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

#include "ipc/action_command.h"

#include "actions/action_executor.h"
#include "device.h"
#include "device/value_getter.h"
#include "http/httprequestqueue.h"
#include "user.h"

supla_action_command::supla_action_command(
    supla_abstract_ipc_socket_adapter *socket_adapter, int action)
    : supla_abstract_action_command(socket_adapter, action) {}

bool supla_action_command::action_open_close(
    int user_id, int device_id, int channel_id, bool open,
    const char *alexa_correlation_token, const char *google_request_id) {
  bool result = false;
  supla_user::access_device(
      user_id, device_id, channel_id,
      [&result, channel_id, open, this](supla_device *device) -> void {
        // onChannelValueChangeEvent must be called before
        // set_device_channel_char_value for the potential report to contain
        // AlexaCorrelationToken / GoogleRequestId
        supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
            device->getUser(), device->getID(), channel_id, get_caller(),
            get_alexa_correlation_token(), get_google_request_id());

        if (open) {
          result = device->get_channels()->action_open(get_caller(), channel_id,
                                                       0, 0);
        } else {
          result =
              device->get_channels()->action_close(get_caller(), channel_id);
        }
      });

  return result;
}

bool supla_action_command::action_toggle(int user_id, int device_id,
                                         int channel_id) {
  bool result = false;
  supla_user::access_device(
      user_id, device_id, channel_id,
      [&result, channel_id, this](supla_device *device) -> void {
        result = device->get_channels()->action_toggle(get_caller(), channel_id,
                                                       0, 0);
      });

  return result;
}

bool supla_action_command::action_stop(int user_id, int device_id,
                                       int channel_id) {
  bool result = false;
  supla_user::access_device(
      user_id, device_id, channel_id,
      [&result, channel_id, this](supla_device *device) -> void {
        result =
            device->get_channels()->action_stop(get_caller(), channel_id, 0, 0);
      });

  return result;
}

bool supla_action_command::action_up_or_stop(int user_id, int device_id,
                                             int channel_id) {
  bool result = false;
  supla_user::access_device(
      user_id, device_id, channel_id,
      [&result, channel_id, this](supla_device *device) -> void {
        result = device->get_channels()->action_up_or_stop(get_caller(),
                                                           channel_id, 0, 0);
      });

  return result;
}

bool supla_action_command::action_down_or_stop(int user_id, int device_id,
                                               int channel_id) {
  bool result = false;
  supla_user::access_device(
      user_id, device_id, channel_id,
      [&result, channel_id, this](supla_device *device) -> void {
        result = device->get_channels()->action_down_or_stop(get_caller(),
                                                             channel_id, 0, 0);
      });

  return result;
}

bool supla_action_command::action_step_by_step(int user_id, int device_id,
                                               int channel_id) {
  bool result = false;
  supla_user::access_device(
      user_id, device_id, channel_id,
      [&result, channel_id, this](supla_device *device) -> void {
        result = device->get_channels()->action_step_by_step(get_caller(),
                                                             channel_id, 0, 0);
      });

  return result;
}

bool supla_action_command::action_copy(int user_id, int device_id,
                                       int channel_id, int source_device_id,
                                       int source_channel_id) {
  bool result = false;

  supla_action_executor *action_executor = new supla_action_executor();
  action_executor->set_caller(get_caller());
  if (action_executor) {
    action_executor->set_channel_id(user_id, device_id, channel_id);

    supla_value_getter *value_getter = new supla_value_getter();
    if (value_getter) {
      action_executor->copy(value_getter, source_device_id, source_channel_id);
      result = true;
      delete value_getter;
      value_getter = NULL;
    }

    delete action_executor;
    action_executor = NULL;
  }
  return result;
}
