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

#ifndef CHANNEL_IC_VALUE_H_
#define CHANNEL_IC_VALUE_H_

#include "device/value/channel_value.h"
#include "proto.h"

class supla_channel_ic_value : public supla_channel_value {
 private:
  char original_raw_value[SUPLA_CHANNELVALUE_SIZE];

 public:
  supla_channel_ic_value(void);
  explicit supla_channel_ic_value(
      const char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  explicit supla_channel_ic_value(const TDS_ImpulseCounter_Value *value);
  virtual void set_raw_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  const TDS_ImpulseCounter_Value *get_ic_value(void);
  virtual void apply_channel_properties(int type,
                                        unsigned char protocol_version,
                                        int param1, int param2, int param3,
                                        int param4,
                                        channel_json_config *json_config);
  virtual supla_channel_extended_value *convert2extended(
      channel_json_config *json_config, int func, const char *text_param1,
      const char *text_param2, int param2, int param3,
      supla_channel_extended_value **data_logger_purpose);
  static bool is_function_supported(int func);
};

#endif /*CHANNEL_IC_VALUE_H_*/
