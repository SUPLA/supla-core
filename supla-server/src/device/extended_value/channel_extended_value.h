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

#ifndef CHANNEL_EXTENDED_VALUE_H_
#define CHANNEL_EXTENDED_VALUE_H_

#include <stddef.h>

#include "proto.h"

class supla_channel_extended_value {
 private:
  size_t real_size = 0;
  TSuplaChannelExtendedValue
      *value;  // Attention! This variable may be allocated a smaller size than
               // specified by sizeof.

 protected:
  TSuplaChannelExtendedValue *get_value_ptr(size_t required_data_size);
  const TSuplaChannelExtendedValue *get_value_ptr(void);
  size_t get_real_size(void);

 public:
  supla_channel_extended_value(void);
  explicit supla_channel_extended_value(TSuplaChannelExtendedValue *value);
  explicit supla_channel_extended_value(supla_channel_extended_value *value);
  virtual ~supla_channel_extended_value(void);
  virtual bool is_differ(supla_channel_extended_value *value);
  void get_raw_value(TSuplaChannelExtendedValue *value);
  void set_raw_value(const TSuplaChannelExtendedValue *value);
};

#endif /*CHANNEL_EXTENDED_VALUE_H_*/
