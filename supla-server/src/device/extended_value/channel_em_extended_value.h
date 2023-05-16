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

#ifndef CHANNEL_EM_EXTENDED_VALUE_H_
#define CHANNEL_EM_EXTENDED_VALUE_H_

#include <string>

#include "device/extended_value/channel_billing_value.h"
#include "device/extended_value/channel_extended_value.h"

class channel_json_config;
class supla_voltage_analyzers;
class supla_channel_em_extended_value : public supla_channel_extended_value,
                                        private supla_channel_billing_value {
 private:
  void set_raw_value(const TElectricityMeter_ExtendedValue_V2 *_value,
                     const char *text_param1, int *param2);

 public:
  supla_channel_em_extended_value(const TSuplaChannelExtendedValue *value,
                                  const char *text_param1, int param2);
  supla_channel_em_extended_value(
      const TElectricityMeter_ExtendedValue_V2 *value, const char *text_param1,
      int param2);
  virtual ~supla_channel_em_extended_value(void);

  std::string get_currency(void);
  double get_total_cost(void);
  double get_total_cost_balanced(void);
  double get_price_per_unit(void);
  _supla_int_t get_measured_values(void);

  virtual bool get_raw_value(TSuplaChannelExtendedValue *value);
  virtual bool get_raw_value(TElectricityMeter_ExtendedValue_V2 *value);
  virtual void set_raw_value(const TSuplaChannelExtendedValue *value);
  virtual void set_raw_value(const TElectricityMeter_ExtendedValue_V2 *value);

  static bool is_function_supported(int func);
  static bool is_ev_type_supported(char type);
  virtual supla_channel_extended_value *copy(void);  // NOLINT
};

#endif /*CHANNEL_EM_EXTENDED_VALUE_H_*/
