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

#include "abstract_value_getter.h"

#include <string.h>

supla_abstract_value_getter::supla_abstract_value_getter() {
  this->user_id = 0;
  this->device_id = 0;
  this->channel_id = 0;
}
supla_abstract_value_getter::supla_abstract_value_getter(int user_id,
                                                         int device_id,
                                                         int channel_id) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;
}

supla_abstract_value_getter::~supla_abstract_value_getter() {}

supla_channel_value* supla_abstract_value_getter::get_value(void) {
  if (!user_id || (!device_id && !channel_id)) {
    return NULL;
  }

  return _get_value(user_id, device_id, channel_id);
}

supla_channel_value* supla_abstract_value_getter::get_value(int user_id,
                                                            int device_id,
                                                            int channel_id) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;

  return get_value();
}

int supla_abstract_value_getter::get_user_id(void) { return user_id; }

int supla_abstract_value_getter::get_device_id(void) { return device_id; }

int supla_abstract_value_getter::get_channel_id(void) { return channel_id; }
