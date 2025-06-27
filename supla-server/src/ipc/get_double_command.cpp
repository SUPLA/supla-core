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

#include "ipc/get_double_command.h"

#include <memory>

#include "device/channel_property_getter.h"
#include "device/value/channel_floating_point_sensor_value.h"

using std::shared_ptr;

supla_get_double_command::supla_get_double_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_get_double_command(socket_adapter) {}

bool supla_get_double_command::get_channel_double_value(int user_id,
                                                        int device_id,
                                                        int channel_id,
                                                        double *value) {
  supla_channel_property_getter getter;
  supla_channel_floating_point_sensor_value *fpv =
      getter.get_value_as<supla_channel_floating_point_sensor_value>(
          user_id, device_id, channel_id);

  if (fpv) {
    *value = fpv->get_value();
    delete fpv;
    return true;
  }

  return false;
}
