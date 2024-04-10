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

#ifndef SHADING_SYSTEM_BASE_CONFIG_H_
#define SHADING_SYSTEM_BASE_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class shading_system_base_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;

 protected:
  std::map<unsigned _supla_int16_t, std::string> get_field_map(void);

  void set_config(_supla_int_t closing_time_ms, _supla_int_t opening_time_ms,
                  unsigned char motor_upside_down,
                  unsigned char buttons_upside_down, signed char time_margin);

  bool get_config(_supla_int_t *closing_time_ms, _supla_int_t *opening_time_ms,
                  unsigned char *motor_upside_down,
                  unsigned char *buttons_upside_down, signed char *time_margin);

 public:
  explicit shading_system_base_config(supla_json_config *root);
  shading_system_base_config(void);
};

#endif /* SHADING_SYSTEM_BASE_CONFIG_H_ */
