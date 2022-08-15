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
#include "devicecontainer.h"

#include "device/device.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;
using std::vector;

supla_user_device_container::supla_user_device_container()
    : supla_connection_objects() {}

supla_user_device_container::~supla_user_device_container() {}

bool supla_user_device_container::add(shared_ptr<supla_device> device) {
  return supla_connection_objects::add(device);
}

shared_ptr<supla_device> supla_user_device_container::find_by_id(
    int device_id) {
  return dynamic_pointer_cast<supla_device>(
      supla_connection_objects::find_by_id(device_id));
}

shared_ptr<supla_device> supla_user_device_container::find_by_guid(
    const char *guid) {
  return dynamic_pointer_cast<supla_device>(
      supla_connection_objects::find_by_guid(guid));
}

shared_ptr<supla_device> supla_user_device_container::find_by_channel_id(
    int channel_id) {
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

vector<shared_ptr<supla_device> > supla_user_device_container::get_all(void) {
  vector<shared_ptr<supla_device> > result;

  for_each([&result](shared_ptr<supla_connection_object> obj) -> bool {
    result.push_back(dynamic_pointer_cast<supla_device>(obj));
    return true;
  });

  return result;
}

bool supla_user_device_container::get_channel_double_value(int device_id,
                                                           int channel_id,
                                                           double *value,
                                                           char type) {
  bool result = false;

  shared_ptr<supla_device> device = find_by_id(device_id);
  if (device != nullptr) {
    supla_device_channels *channels = device->get_channels();
    switch (type) {
      case 0:
        result = channels->get_channel_double_value(channel_id, value) == 1;
        break;
      case 1:
        result =
            channels->get_channel_temperature_value(channel_id, value) == 1;
        break;
      case 2:
        result = channels->get_channel_humidity_value(channel_id, value) == 1;
        break;
    }
  }

  return result;
}

bool supla_user_device_container::get_channel_char_value(int device_id,
                                                         int channel_id,
                                                         char *value) {
  bool result = false;

  shared_ptr<supla_device> device = find_by_id(device_id);

  if (device != nullptr) {
    result =
        device->get_channels()->get_channel_char_value(channel_id, value) == 1;
  }

  return result;
}

bool supla_user_device_container::get_channel_rgbw_value(
    int device_id, int channel_id, int *color, char *color_brightness,
    char *brightness, char *on_off) {
  bool result = false;
  shared_ptr<supla_device> device = find_by_id(device_id);

  if (device != nullptr) {
    result = device->get_channels()->get_channel_rgbw_value(
                 channel_id, color, color_brightness, brightness, on_off) == 1;
  }

  return result;
}

bool supla_user_device_container::get_channel_valve_value(int device_id,
                                                          int channel_id,
                                                          TValve_Value *value) {
  bool result = false;
  shared_ptr<supla_device> device = find_by_id(device_id);

  if (device != nullptr) {
    result =
        device->get_channels()->get_channel_valve_value(channel_id, value) == 1;
  }

  return result;
}

void supla_user_device_container::set_channel_function(int channel_id,
                                                       int func) {
  shared_ptr<supla_device> device = find_by_channel_id(channel_id);
  if (device != nullptr) {
    device->get_channels()->set_channel_function(channel_id, func);
  }
}
