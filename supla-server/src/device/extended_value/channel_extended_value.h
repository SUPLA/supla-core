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
#include <map>
#include <string>

#include "proto.h"

class supla_channel_extended_value {
 private:
  size_t real_size = 0;
  TSuplaChannelExtendedValue
      *value;  // Attention! This variable may be allocated a smaller size than
               // specified by sizeof.

 protected:
  TSuplaChannelExtendedValue *_realloc(size_t required_data_size);
  TSuplaChannelExtendedValue *get_value_ptr(void);
  virtual void set_raw_value(const TSuplaChannelExtendedValue *value);

 public:
  supla_channel_extended_value(void);
  explicit supla_channel_extended_value(
      const TSuplaChannelExtendedValue *value);
  explicit supla_channel_extended_value(
      const supla_channel_extended_value *value);
  virtual ~supla_channel_extended_value(void);
  char get_type();
  virtual supla_channel_extended_value *copy(void);  // NOLINT
  virtual size_t get_real_size(void);
  virtual size_t get_value_size(void);
  virtual bool is_differ(supla_channel_extended_value *value);
  virtual bool get_raw_value(TSuplaChannelExtendedValue *value);
  void get_value(char *buffer);
  virtual std::map<std::string, std::string> get_replacement_map(void);
};

#endif /*CHANNEL_EXTENDED_VALUE_H_*/
