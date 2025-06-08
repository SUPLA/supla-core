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

#ifndef CHANNEL_VALVE_VALUE_H_
#define CHANNEL_VALVE_VALUE_H_

#include <map>
#include <string>

#include "device/value/abstract_channel_value.h"

class supla_channel_valve_value : public supla_abstract_channel_value {
 public:
  supla_channel_valve_value();
  explicit supla_channel_valve_value(
      const char raw_value[SUPLA_CHANNELVALUE_SIZE]);
  explicit supla_channel_valve_value(const TValve_Value *value);
  virtual supla_abstract_channel_value *copy(void) const;  // NOLINT
  const TValve_Value *get_valve_value(void);
  void get_valve_value(TValve_Value *value);
  void set_valve_value(TValve_Value *value);
  virtual std::map<std::string, std::string> get_replacement_map(void);
  static bool is_function_supported(int func);
  virtual bool get_vbt_value(_vbt_var_name_e var_name, double *value);
};

#endif /*CHANNEL_VALVE_VALUE_H_*/
