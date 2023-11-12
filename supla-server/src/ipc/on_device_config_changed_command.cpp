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

#include "on_device_config_changed_command.h"

#include <memory>

#include "device/device.h"
#include "user/user.h"
#include "user/userdevices.h"

using std::shared_ptr;

supla_on_device_config_changed_command::supla_on_device_config_changed_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_on_device_config_changed_command(socket_adapter) {}

void supla_on_device_config_changed_command::on_device_config_changed(
    int user_id, int device_id) {
  supla_user *user = supla_user::find(user_id, false);
  if (!user) {
    return;
  }

  shared_ptr<supla_device> device = user->get_devices()->get(device_id);
  if (device) {
    device->send_config_to_device();
  }

  user->get_clients()->for_each([device_id](shared_ptr<supla_client> client,
                                            bool *will_continue) -> void {
    client->send_device_config(device_id, 0xFFFFFFFFFFFFFFFF);
  });
}
