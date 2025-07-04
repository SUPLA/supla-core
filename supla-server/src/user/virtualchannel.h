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

#ifndef VIRTUAL_CHANNEL_H_
#define VIRTUAL_CHANNEL_H_

#include "device/channel_availability_status.h"
#include "proto.h"

class supla_abstract_channel_value;
class supla_user;
class supla_virtual_channel {
 private:
  int device_id;
  int channel_id;
  int type;
  int func;
  supla_abstract_channel_value *value;
  struct timeval value_valid_to;

 public:
  supla_virtual_channel(void);
  supla_virtual_channel(const supla_virtual_channel &channel);
  supla_virtual_channel(supla_user *user, int device_id, int channe_id,
                        const char raw_value[SUPLA_CHANNELVALUE_SIZE],
                        const unsigned _supla_int_t validity_time_sec, int type,
                        int func, int param1, int param2, int param3,
                        int param4, const char *user_config,
                        const char *properties);
  virtual ~supla_virtual_channel();
  static unsigned get_value_validity_time_sec(
      const struct timeval &value_valid_to);
  unsigned get_value_validity_time_sec(void);
  int get_device_id(void);
  int get_channel_id(void);
  int get_func(void);
  int get_type(void);
  const supla_abstract_channel_value *get_value(void);
  bool get_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  supla_channel_availability_status get_availability_status(void);
  bool apply_changes(supla_user *user, const supla_virtual_channel *channel);
  supla_virtual_channel &operator=(const supla_virtual_channel &channel);
};

#endif /* VIRTUAL_CHANNEL_H_ */
