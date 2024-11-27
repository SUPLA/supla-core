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

#ifndef CHANNEL_GENERAL_PURPOSE_BASE_VALUE_H_
#define CHANNEL_GENERAL_PURPOSE_BASE_VALUE_H_

#include <string>

#include "device/value/channel_value.h"

class supla_channel_general_purpose_base_value : public supla_channel_value {
 private:
  std::string value_to_str(double value, std::string *nan);

 public:
  supla_channel_general_purpose_base_value();
  explicit supla_channel_general_purpose_base_value(
      const char raw_value[SUPLA_CHANNELVALUE_SIZE]);

  virtual void apply_channel_properties(int type,
                                        unsigned char protocol_version,
                                        int param1, int param2, int param3,
                                        int param4,
                                        supla_json_config *json_config);

  virtual double get_value(void);
  virtual void set_value(double value);
  std::string get_value_str();
  std::string get_value_str(std::string nan);
  virtual bool get_vbt_value(_vbt_var_name_e var_name, double *value);
};

#endif /*CHANNEL_GENERAL_PURPOSE_BASE_VALUE_H_*/
