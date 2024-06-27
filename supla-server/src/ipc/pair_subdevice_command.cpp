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

#include "ipc/pair_subdevice_command.h"

#include <memory>

#include "device.h"
#include "user.h"

using std::shared_ptr;

supla_pair_subdevice_command::supla_pair_subdevice_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_pair_subdevice_command(socket_adapter) {}

bool supla_pair_subdevice_command::pair_subdevice(int user_id, int device_id) {
  shared_ptr<supla_device> device =
      supla_user::get_device(user_id, device_id, 0);
  if (device != nullptr) {
    return device->pair_subdevice(get_caller());
  }
  return false;
}
