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

#ifndef ABSTRACT_RS_VALUE_H_
#define ABSTRACT_RS_VALUE_H_

#include "device/value/abstract_channel_value.h"
#include "user/user.h"

class supla_abstract_rs_value : public supla_abstract_channel_value {
 public:
  supla_abstract_rs_value(void);
  explicit supla_abstract_rs_value(
      const char raw_value[SUPLA_CHANNELVALUE_SIZE]);

  bool is_calibration_failed(void);
  bool is_calibration_lost(void);
  bool is_motor_problem(void);
  bool is_calibration_in_progress(void);
  bool is_any_error_set(void);
  virtual char get_position(void) = 0;
  virtual _supla_int16_t get_flags(void) = 0;
  virtual bool get_vbt_value(_vbt_var_name_e var_name, double *value);
  virtual nlohmann::json get_template_data(void);
};

#endif /*ABSTRACT_RS_VALUE_H_*/
