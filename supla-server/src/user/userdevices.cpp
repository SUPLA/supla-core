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
using std::function;
using std::shared_ptr;
using std::vector;
using std::weak_ptr;

supla_user_devices::supla_user_devices() : supla_connection_objects() {}

supla_user_devices::~supla_user_devices() {}

bool supla_user_devices::add(shared_ptr<supla_device> device) {
  bool result = supla_connection_objects::add(device);

  vector<supla_channel_fragment> fragments =
      device->get_channels()->get_fragments();

  int device_id = device->get_id();

  lock();
  for (auto it = channel_fragments.begin(); it != channel_fragments.end();
       ++it) {
    if (it->get_device_id() == device_id) {
      it = channel_fragments.erase(it);
      --it;
    }
  }

  for (auto it = fragments.rbegin(); it != fragments.rend(); ++it) {
    channel_fragments.push_back(*it);
  }
  unlock();

  return result;
}

std::shared_ptr<supla_device> supla_user_devices::get(int device_id) {
  return dynamic_pointer_cast<supla_device>(
      supla_connection_objects::get(device_id));
}

std::shared_ptr<supla_device> supla_user_devices::get(int device_id,
                                                      int channel_id) {
  if (device_id) {
    return get(device_id);
  } else if (channel_id) {
    shared_ptr<supla_device> result;

    for_each([&result, channel_id](shared_ptr<supla_device> device,
                                   bool *will_continue) -> void {
      if (device->get_channels()->channel_exists(channel_id)) {
        result = device;
        *will_continue = false;
      }
    });

    return result;
  }

  return nullptr;
}

supla_channel_fragment supla_user_devices::get_channel_fragment(
    int channel_id) {
  supla_channel_fragment result;
  lock();
  for (auto it = channel_fragments.rbegin(); it != channel_fragments.rend();
       ++it) {
    if (it->get_channel_id() == channel_id) {
      result = *it;
      break;
    }
  }
  unlock();
  return result;
}

void supla_user_devices::for_each(
    function<void(shared_ptr<supla_device> device, bool *will_continue)>
        on_device) {
  supla_connection_objects::for_each(
      [on_device](shared_ptr<supla_abstract_connection_object> obj,
                  bool *will_continue) -> void {
        shared_ptr<supla_device> device =
            dynamic_pointer_cast<supla_device>(obj);
        if (device) {
          on_device(device, will_continue);
        }
      });
}
