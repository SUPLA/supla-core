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
#include <userdevices.h>

#include "device/device.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;
using std::vector;

supla_user_devices::supla_user_devices() : supla_connection_objects() {}

supla_user_devices::~supla_user_devices() {}

bool supla_user_devices::add(shared_ptr<supla_device> device) {
  return supla_connection_objects::add(device);
}

shared_ptr<supla_device> supla_user_devices::find_by_id(int device_id) {
  return dynamic_pointer_cast<supla_device>(
      supla_connection_objects::find_by_id(device_id));
}

shared_ptr<supla_device> supla_user_devices::find_by_channel_id(
    int channel_id) {
  if (!channel_id) {
    return nullptr;
  }

  shared_ptr<supla_device> result;

  for_each([&result,
            channel_id](shared_ptr<supla_connection_object> obj) -> bool {
    shared_ptr<supla_device> device = dynamic_pointer_cast<supla_device>(obj);
    if (device->get_channels()->channel_exists(channel_id)) {
      result = device;
      return false;
    }
    return true;
  });

  return result;
}

std::shared_ptr<supla_device> supla_user_devices::get(int device_id) {
  return dynamic_pointer_cast<supla_device>(
      supla_connection_objects::find_by_id(device_id));
}

std::shared_ptr<supla_device> supla_user_devices::get(int device_id,
                                                      int channel_id) {
  if (device_id || channel_id) {
    shared_ptr<supla_device> device =
        device_id ? find_by_id(device_id) : find_by_channel_id(channel_id);
    return device;
  }
  return nullptr;
}

vector<shared_ptr<supla_device> > supla_user_devices::get_all(void) {
  vector<shared_ptr<supla_device> > result;

  for_each([&result](shared_ptr<supla_connection_object> obj) -> bool {
    result.push_back(dynamic_pointer_cast<supla_device>(obj));
    return true;
  });

  return result;
}
