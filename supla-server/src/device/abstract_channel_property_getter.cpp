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

#include "device/abstract_channel_property_getter.h"

#include <string.h>

supla_abstract_channel_property_getter::
    supla_abstract_channel_property_getter() {
  this->user_id = 0;
  this->device_id = 0;
  this->channel_id = 0;
}
supla_abstract_channel_property_getter::supla_abstract_channel_property_getter(
    int user_id, int device_id, int channel_id) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;
}

supla_abstract_channel_property_getter::
    ~supla_abstract_channel_property_getter() {}

supla_abstract_channel_value* supla_abstract_channel_property_getter::get_value(
    supla_channel_fragment* fragment,
    supla_channel_availability_status* status) {
  if (!user_id || (!device_id && !channel_id)) {
    return nullptr;
  }

  supla_abstract_channel_value* result =
      _get_value(user_id, device_id, channel_id, fragment, status);
  if (result == nullptr && status) {
    status->set_offline(true);
  }

  return result;
}

supla_abstract_channel_value* supla_abstract_channel_property_getter::get_value(
    int* func, supla_channel_availability_status* status) {
  supla_channel_fragment fragment;
  supla_abstract_channel_value* result =
      get_value(func ? &fragment : nullptr, status);

  if (func) {
    *func = fragment.get_function();
  }

  return result;
}

supla_abstract_channel_value* supla_abstract_channel_property_getter::get_value(
    void) {
  return get_value((supla_channel_fragment*)nullptr, nullptr);
}

supla_abstract_channel_value* supla_abstract_channel_property_getter::get_value(
    int user_id, int device_id, int channel_id, int* func,
    supla_channel_availability_status* status) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;

  return get_value(func, status);
}

supla_abstract_channel_value* supla_abstract_channel_property_getter::get_value(
    int user_id, int device_id, int channel_id,
    supla_channel_fragment* fragment,
    supla_channel_availability_status* status) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;

  return get_value(fragment, status);
}

supla_abstract_channel_value* supla_abstract_channel_property_getter::get_value(
    int user_id, int device_id, int channel_id) {
  return get_value(user_id, device_id, channel_id,
                   (supla_channel_fragment*)nullptr, nullptr);
}

int supla_abstract_channel_property_getter::get_func(void) {
  return _get_func(user_id, device_id, channel_id);
}

int supla_abstract_channel_property_getter::get_func(int user_id, int device_id,
                                                     int channel_id) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;
  return get_func();
}

int supla_abstract_channel_property_getter::get_channel_id(
    int user_id, int device_id, unsigned char channel_number) {
  if (user_id && device_id) {
    return _get_channel_id(user_id, device_id, channel_number);
  }

  return 0;
}

int supla_abstract_channel_property_getter::get_channel_id(
    unsigned char channel_number) {
  return get_channel_id(user_id, device_id, channel_number);
}

supla_abstract_channel_extended_value*
supla_abstract_channel_property_getter::get_extended_value(void) {
  return _get_extended_value(user_id, device_id, channel_id);
}

supla_abstract_channel_extended_value*
supla_abstract_channel_property_getter::get_extended_value(int user_id,
                                                           int device_id,
                                                           int channel_id) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;

  return get_extended_value();
}

supla_json_config*
supla_abstract_channel_property_getter::get_detached_json_config(void) {
  return _get_detached_json_config(user_id, device_id, channel_id);
}

supla_json_config*
supla_abstract_channel_property_getter::get_detached_json_config(
    int user_id, int device_id, int channel_id) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;

  return get_detached_json_config();
}

int supla_abstract_channel_property_getter::get_user_id(void) {
  return user_id;
}

int supla_abstract_channel_property_getter::get_device_id(void) {
  return device_id;
}

int supla_abstract_channel_property_getter::get_channel_id(void) {
  return channel_id;
}
