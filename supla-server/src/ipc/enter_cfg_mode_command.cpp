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

#include "ipc/enter_cfg_mode_command.h"

#include "device.h"
#include "user.h"

supla_enter_cfg_mode_command::supla_enter_cfg_mode_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_enter_cfg_mode_command(socket_adapter) {}

bool supla_enter_cfg_mode_command::enter_cfg_mode(int user_id, int device_id) {
  int result = false;

  supla_user::access_device(user_id, device_id, 0,
                            [&result, this](supla_device *device) -> void {
                              result = device->enter_cfg_mode();
                            });

  return result;
}
