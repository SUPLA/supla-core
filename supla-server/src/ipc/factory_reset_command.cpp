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

#include "factory_reset_command.h"

#include <memory>

#include "device.h"
#include "user.h"

using std::shared_ptr;

supla_factory_reset_command::supla_factory_reset_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_factory_reset_command(socket_adapter) {}

bool supla_factory_reset_command::factory_reset(int user_id, int device_id) {
  supla_user *user = supla_user::get_user(user_id);
  if (user) {
    shared_ptr<supla_device> device = user->get_devices()->get(device_id);
    if (device) {
      return device->factory_reset();
    }
  }

  return false;
}
