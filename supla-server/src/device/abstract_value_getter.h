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

#ifndef ABSTRACT_VALUE_GETTER_H_
#define ABSTRACT_VALUE_GETTER_H_

#include "device/channel_value.h"
#include "proto.h"

class supla_abstract_value_getter {
 private:
  int user_id;
  int device_id;
  int channel_id;

 protected:
  virtual supla_channel_value* _get_value(int user_id, int device_id,
                                          int channel_id) = 0;

 public:
  supla_abstract_value_getter();
  supla_abstract_value_getter(int user_id, int device_id, int channel_id);
  virtual ~supla_abstract_value_getter();

  supla_channel_value* get_value(void);
  supla_channel_value* get_value(int user_id, int device_id, int channel_id);
  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);
};

#endif /* ABSTRACT_VALUE_GETTER_H_ */
