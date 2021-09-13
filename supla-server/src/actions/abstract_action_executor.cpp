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

#include <actions/abstract_action_executor.h>

supla_abstract_action_executor::supla_abstract_action_executor(void) {
  this->user = 0;
  this->device_id = 0;
  this->channel_id = 0;
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

supla_abstract_action_executor::~supla_abstract_action_executor(void) {}

void supla_abstract_action_executor::set_channel_id(supla_user *user,
                                                    int device_id,
                                                    int channel_id) {
  this->user = user;
  this->device_id = device_id;
  this->channel_id = channel_id;
}

void supla_abstract_action_executor::set_channel_id(int user_id, int device_id,
                                                    int channel_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = device_id;
  this->channel_id = channel_id;
}

supla_device *supla_abstract_action_executor::get_device(void) {
  if (user && device_id) {
    return user->get_device(device_id);
  }

  return NULL;
}

int supla_abstract_action_executor::get_channel_id(void) { return channel_id; }
