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

#include "ipc/get_channel_state_command.h"

#include <memory>

#include "db/db_access_provider.h"
#include "device/device_dao.h"
#include "device/extended_value/channel_and_timer_state_extended_value.h"
#include "device/extended_value/channel_state_extended_value.h"
#include "user.h"

using std::shared_ptr;

supla_get_channel_state_command::supla_get_channel_state_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_get_channel_state_command(socket_adapter) {}

bool supla_get_channel_state_command::get_channel_state(
    TDSC_ChannelState *state, int user_id, int device_id, int channel_id,
    bool send_request, bool get_from_extended_value) {
  bool result = false;

  if (get_from_extended_value) {
    supla_db_access_provider dba;
    supla_device_dao dao(&dba);

    supla_channel_extended_value *ev =
        dao.get_channel_extended_value(user_id, channel_id);
    if (ev) {
      if (dynamic_cast<supla_channel_state_extended_value *>(ev)) {
        dynamic_cast<supla_channel_state_extended_value *>(ev)->get_raw_value(
            state);
        result = true;
      } else if (dynamic_cast<supla_channel_and_timer_state_extended_value *>(
                     ev)) {
        dynamic_cast<supla_channel_and_timer_state_extended_value *>(ev)
            ->get_raw_value(state);
        result = true;
      }
      delete ev;
    }
  }

  if (send_request || !get_from_extended_value) {
    shared_ptr<supla_device> device =
        supla_user::get_device(user_id, device_id, channel_id);
    if (device != nullptr) {
      if (!get_from_extended_value) {
        result = device->get_channels()->get_channel_state(channel_id, state);
      }
      if (send_request) {
        TCSD_ChannelStateRequest request = {};
        request.ChannelID = channel_id;

        device->get_channels()->get_channel_state_async(supla_caller(ctIPC),
                                                        &request);
      }
    }
  }

  return result;
}
