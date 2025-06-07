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

#include "ipc/get_valve_value_command.h"

#include "device/channel_property_getter.h"
#include "device/value/channel_valve_value.h"

supla_get_valve_value_command::supla_get_valve_value_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_get_valve_value_command(socket_adapter) {}

bool supla_get_valve_value_command::get_channel_valve_value(
    int user_id, int device_id, int channel_id, TValve_Value *value) {
  bool result = false;

  supla_channel_property_getter getter;
  supla_channel_valve_value *vv =
      getter.get_value_as<supla_channel_valve_value>(user_id, device_id,
                                                     channel_id);

  if (vv) {
    vv->get_valve_value(value);
    delete vv;
    result = true;
  }

  return result;
}
