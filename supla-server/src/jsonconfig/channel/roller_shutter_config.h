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

#ifndef ROLLER_SHUTTER_CONFIG_H_
#define ROLLER_SHUTTER_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"

class roller_shutter_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  std::string time_margin_to_string(char time_margin);
  char string_to_time_margin(const std::string &tm);

 protected:
  std::map<unsigned _supla_int16_t, std::string> get_field_map(void);

  void set_config(_supla_int_t closing_time_ms, _supla_int_t opening_time_ms,
                  unsigned char motor_upside_down,
                  unsigned char buttons_upside_down, signed char time_margin);

  bool get_config(_supla_int_t *closing_time_ms, _supla_int_t *opening_time_ms,
                  unsigned char *motor_upside_down,
                  unsigned char *buttons_upside_down, signed char *time_margin);

 public:
  explicit roller_shutter_config(supla_json_config *root);
  roller_shutter_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_RollerShutter *config);
  bool get_config(TChannelConfig_RollerShutter *config);
  unsigned int get_value_duration(void);
};

#endif /* ROLLER_SHUTTER_CONFIG_H_ */
