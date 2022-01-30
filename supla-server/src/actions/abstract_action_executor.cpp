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

#include "actions/abstract_action_executor.h"

#include "converter/any_value_to_action_converter.h"

supla_abstract_action_executor::supla_abstract_action_executor(void) {
  this->user = NULL;
  this->device_id = 0;
  this->subject_id = 0;
  this->is_group = false;
}

supla_abstract_action_executor::supla_abstract_action_executor(supla_user *user,
                                                               int device_id,
                                                               int channel_id) {
  set_channel_id(user, device_id, channel_id);
}

supla_abstract_action_executor::supla_abstract_action_executor(int user_id,
                                                               int device_id,
                                                               int channel_id) {
  set_channel_id(user_id, device_id, channel_id);
}

supla_abstract_action_executor::supla_abstract_action_executor(supla_user *user,
                                                               int group_id) {
  set_group_id(user, group_id);
}

supla_abstract_action_executor::supla_abstract_action_executor(int user_id,
                                                               int group_id) {
  set_group_id(user_id, group_id);
}

supla_abstract_action_executor::~supla_abstract_action_executor(void) {}

void supla_abstract_action_executor::set_channel_id(supla_user *user,
                                                    int device_id,
                                                    int channel_id) {
  this->user = user;
  this->device_id = device_id;
  this->subject_id = channel_id;
  this->is_group = false;
}

void supla_abstract_action_executor::set_channel_id(int user_id, int device_id,
                                                    int channel_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = device_id;
  this->subject_id = channel_id;
  this->is_group = false;
}

void supla_abstract_action_executor::set_group_id(supla_user *user,
                                                  int group_id) {
  this->user = user;
  this->device_id = 0;
  this->subject_id = group_id;
  this->is_group = true;
}

void supla_abstract_action_executor::set_group_id(int user_id, int group_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = 0;
  this->subject_id = group_id;
  this->is_group = true;
}

void supla_abstract_action_executor::access_device(
    std::function<void(supla_device *device)> on_device) {
  if (user && device_id) {
    return user->access_device(device_id, 0, on_device);
  }
}

supla_user_channelgroups *supla_abstract_action_executor::get_channel_groups(
    void) {
  return is_group && user ? user->get_channel_groups() : NULL;
}

supla_user *supla_abstract_action_executor::get_user(void) { return user; }

int supla_abstract_action_executor::get_user_id(void) {
  return user ? user->getUserID() : 0;
}

int supla_abstract_action_executor::get_device_id(void) { return device_id; }

int supla_abstract_action_executor::get_channel_id(void) {
  return is_group ? 0 : subject_id;
}

int supla_abstract_action_executor::get_group_id(void) {
  return is_group ? subject_id : 0;
}

void supla_abstract_action_executor::copy(
    supla_abstract_value_getter *value_getter, int sourceDeviceId,
    int sourceChannelId) {
  if (value_getter) {
    char value[SUPLA_CHANNELVALUE_SIZE] = {};
    int channelFunc = 0;
    if (value_getter->get_value(get_user_id(), sourceDeviceId, sourceChannelId,
                                value, &channelFunc)) {
      any_value_to_action_converter *converter =
          new any_value_to_action_converter();
      if (converter) {
        converter->convert(value, channelFunc, this);
        delete converter;
      }
    }
  }
}
