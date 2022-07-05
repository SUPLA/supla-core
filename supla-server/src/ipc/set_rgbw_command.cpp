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

#include "device.h"
#include "http/httprequestqueue.h"
#include "user.h"

supla_set_rgbw_command::supla_set_rgbw_command(
    supla_abstract_ipc_socket_adapter *socket_adapter, bool random_color)
    : supla_abstract_set_rgbw_command(socket_adapter, random_color) {}

bool supla_set_rgbw_command::set_channel_rgbw_value(
    supla_user *user, int device_id, int channel_id, int color,
    char color_brightness, char brightness, char on_off,
    const char *alexa_correlation_token, const char *google_request_id) {
  bool result = false;
  user->access_device(
      device_id, channel_id,
      [&result, user, device_id, channel_id, color, color_brightness,
       brightness, on_off, alexa_correlation_token, google_request_id,
       this](supla_device *device) -> void {
        // onChannelValueChangeEvent must be called before
        // set_device_channel_char_value for the potential report to contain
        // AlexaCorrelationToken / GoogleRequestId
        supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
            user, device_id, channel_id, get_caller(), alexa_correlation_token,
            google_request_id);

        result = device->get_channels()->set_device_channel_rgbw_value(
            get_caller(), channel_id, 0, false, color, color_brightness,
            brightness, on_off);
      });

  return result;
}
