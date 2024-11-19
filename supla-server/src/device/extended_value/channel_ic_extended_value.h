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

#ifndef CHANNEL_IC_EXTENDED_VALUE_H_
#define CHANNEL_IC_EXTENDED_VALUE_H_

#include <string>

#include "device/extended_value/channel_billing_value.h"
#include "device/extended_value/channel_extended_value.h"
#include "jsonconfig/json_config.h"

class supla_channel_ic_extended_value : public supla_channel_extended_value,
                                        private supla_channel_billing_value {
 public:
  explicit supla_channel_ic_extended_value(
      const TSuplaChannelExtendedValue *value);
  supla_channel_ic_extended_value(int func,
                                  const TDS_ImpulseCounter_Value *ic_val,
                                  supla_json_config *json_config);
  virtual ~supla_channel_ic_extended_value(void);
  virtual void set_raw_value(const TSuplaChannelExtendedValue *value);
  _supla_int_t get_total_cost(void);
  _supla_int_t get_price_per_unit(void);
  std::string get_currency(void);
  std::string get_custom_unit(void);
  _supla_int_t get_impulses_per_unit(void);
  unsigned _supla_int64_t get_counter(void);
  unsigned _supla_int64_t get_calculated_value(void);
  double get_calculated_value_dbl(void);
  static bool is_function_supported(int func);
  static bool is_ev_type_supported(char type);
  virtual supla_channel_extended_value *copy(void);  // NOLINT
  virtual bool get_vbt_value(_vbt_var_name_e var_name, double *value);
};

#endif /*CHANNEL_IC_EXTENDED_VALUE_H_*/
