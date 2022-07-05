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

#include "ipc/set_char_command.h"

#include "device.h"
#include "http/httprequestqueue.h"
#include "user.h"

supla_set_char_command::supla_set_char_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_set_char_command(socket_adapter) {}

bool supla_set_char_command::set_channel_char_value(
    supla_user *user, int device_id, int channel_id, char value,
    const char *alexa_correlation_token, const char *google_request_id) {
  bool result = false;
  user->access_device(
      device_id, channel_id,
      [&result, channel_id, value, alexa_correlation_token, google_request_id,
       this](supla_device *device) -> void {
        // onChannelValueChangeEvent must be called before
        // set_device_channel_char_value for the potential report to contain
        // AlexaCorrelationToken / GoogleRequestId
        supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
            device->getUser(), device->getID(), channel_id, get_caller(),
            alexa_correlation_token, google_request_id);

        result = device->get_channels()->set_device_channel_char_value(
            supla_caller(ctIPC), channel_id, 0, false, value);
      });

  return result;
}
