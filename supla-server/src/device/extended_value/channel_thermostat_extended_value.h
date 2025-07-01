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

#ifndef CHANNEL_THERMOSTAT_EXTENDED_VALUE_H_
#define CHANNEL_THERMOSTAT_EXTENDED_VALUE_H_

#include "device/extended_value/abstract_channel_extended_value.h"

class supla_channel_thermostat_extended_value
    : public supla_abstract_channel_extended_value {
 public:
  explicit supla_channel_thermostat_extended_value(
      const TSuplaChannelExtendedValue *value);
  explicit supla_channel_thermostat_extended_value(
      const supla_abstract_channel_extended_value *value);
  virtual ~supla_channel_thermostat_extended_value(void);
  short get_flags(unsigned char idx);
  static bool is_ev_type_supported(char type);
  virtual supla_abstract_channel_extended_value *copy(void);  // NOLINT
  virtual bool get_vbt_value(_vbt_var_name_e var_name, double *value);
};

#endif /*CHANNEL_THERMOSTAT_EXTENDED_VALUE_H_*/
