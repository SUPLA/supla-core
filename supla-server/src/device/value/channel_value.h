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

#ifndef CHANNEL_VALUE_H_
#define CHANNEL_VALUE_H_

#include <map>
#include <string>

#include "channeljsonconfig/channel_json_config.h"
#include "device/extended_value/channel_extended_value.h"
#include "proto.h"

class supla_channel_value {
 protected:
  char raw_value[SUPLA_CHANNELVALUE_SIZE];

 public:
  supla_channel_value(void);
  explicit supla_channel_value(const char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  virtual ~supla_channel_value(void);
  void get_raw_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  void set_raw_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  virtual bool is_differ(supla_channel_value *value, bool *significant_change);
  virtual void apply_channel_properties(int type,
                                        unsigned char protocol_version,
                                        int param1, int param2, int param3,
                                        int param4,
                                        channel_json_config *json_config,
                                        _logger_purpose_t *logger_data);
  virtual std::map<std::string, std::string> get_replacement_map(void);
};

#endif /*CHANNEL_VALUE_H_*/
