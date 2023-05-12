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

#ifndef ABSTRACT_PROPERTY_GETTER_H_
#define ABSTRACT_PROPERTY_GETTER_H_

#include "channeljsonconfig/channel_json_config.h"
#include "device/channel_electricity_measurement.h"
#include "device/channel_fragment.h"
#include "device/extended_value/channel_extended_value.h"
#include "device/value/channel_value.h"
#include "proto.h"

class supla_abstract_channel_property_getter {
 private:
  int user_id;
  int device_id;
  int channel_id;

 protected:
  virtual supla_channel_value* _get_value(int user_id, int device_id,
                                          int channel_id,
                                          supla_channel_fragment* fragment,
                                          bool* online) = 0;

  virtual int _get_func(int user_id, int device_id, int channel_id) = 0;
  virtual supla_channel_electricity_measurement* _get_electricity_measurement(
      int user_id, int device_id, int channel_id) = 0;
  virtual supla_channel_extended_value* _get_extended_value(int user_id,
                                                            int device_id,
                                                            int channel_id) = 0;
  virtual channel_json_config* _get_detached_json_config(int user_id,
                                                         int device_id,
                                                         int channel_id) = 0;

 public:
  supla_abstract_channel_property_getter();
  supla_abstract_channel_property_getter(int user_id, int device_id,
                                         int channel_id);
  virtual ~supla_abstract_channel_property_getter();

  supla_channel_value* get_value(void);
  supla_channel_value* get_value(int* func, bool* online);
  supla_channel_value* get_value(supla_channel_fragment* fragment,
                                 bool* online);
  supla_channel_value* get_value(int user_id, int device_id, int channel_id);
  supla_channel_value* get_value(int user_id, int device_id, int channel_id,
                                 int* func, bool* online);
  supla_channel_value* get_value(int user_id, int device_id, int channel_id,
                                 supla_channel_fragment* fragment,
                                 bool* online);
  supla_channel_extended_value* get_extended_value(void);
  supla_channel_extended_value* get_extended_value(int user_id, int device_id,
                                                   int channel_id);

  int get_func(void);
  int get_func(int user_id, int device_id, int channel_id);

  supla_channel_electricity_measurement* get_electricity_measurement(void);
  supla_channel_electricity_measurement* get_electricity_measurement(
      int user_id, int device_id, int channel_id);

  channel_json_config* get_detached_json_config(void);
  channel_json_config* get_detached_json_config(int user_id, int device_id,
                                                int channel_id);

  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);
};

#endif /* ABSTRACT_PROPERTY_GETTER_H_ */
