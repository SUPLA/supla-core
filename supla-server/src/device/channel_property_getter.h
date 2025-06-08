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

#ifndef PROPERTY_GETTER_H_
#define PROPERTY_GETTER_H_

#include "device/abstract_channel_property_getter.h"

class supla_channel_property_getter
    : public supla_abstract_channel_property_getter {
 protected:
  virtual supla_abstract_channel_value *_get_value(
      int user_id, int device_id, int channel_id,
      supla_channel_fragment *fragment,
      supla_channel_availability_status *status);
  virtual int _get_func(int user_id, int device_id, int channel_id);
  virtual supla_channel_extended_value *_get_extended_value(int user_id,
                                                            int device_id,
                                                            int channel_id);
  virtual supla_json_config *_get_detached_json_config(int user_id,
                                                       int device_id,
                                                       int channel_id);

  virtual int _get_channel_id(int user_id, int device_id,
                              unsigned char channel_number);

 public:
  virtual ~supla_channel_property_getter(void);

  virtual supla_channel_availability_status get_channel_availability_status(
      int user_id, int device_id, int channel_id);
};

#endif /*PROPERTY_GETTER_H_*/
