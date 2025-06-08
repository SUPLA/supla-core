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

#ifndef ABSTRACT_CHANNEL_VALUE_H_
#define ABSTRACT_CHANNEL_VALUE_H_

#include <map>
#include <string>

#include "device/extended_value/abstract_channel_extended_value.h"
#include "jsonconfig/json_config.h"
#include "proto.h"
#include "vbt/vbt_value.h"

class supla_abstract_channel_extended_value;
class supla_abstract_channel_value : public supla_vbt_value {
 protected:
  char raw_value[SUPLA_CHANNELVALUE_SIZE];

 public:
  supla_abstract_channel_value(void);
  explicit supla_abstract_channel_value(
      const char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  virtual ~supla_abstract_channel_value(void);
  virtual supla_abstract_channel_value *copy(void) const = 0;  // NOLINT

  void get_raw_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  virtual void set_raw_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  virtual bool is_differ(supla_abstract_channel_value *value,
                         bool *significant_change);
  virtual void apply_channel_properties(int type,
                                        unsigned char protocol_version,
                                        int param1, int param2, int param3,
                                        int param4,
                                        supla_json_config *json_config);
  virtual std::map<std::string, std::string> get_replacement_map(void);
  virtual supla_abstract_channel_extended_value *convert2extended(
      supla_json_config *json_config, int func,
      supla_abstract_channel_extended_value **data_logger_purpose);
};

#endif /*ABSTRACT_CHANNEL_VALUE_H_*/
