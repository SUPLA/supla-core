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

#ifndef GENERAL_PURPOSE_BASE_CONFIG_H_
#define GENERAL_PURPOSE_BASE_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class general_purpose_base_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;

 protected:
  std::map<unsigned _supla_int16_t, std::string> get_field_map(void);

  void set_config(
      _supla_int_t value_divider, _supla_int_t value_multiplier,
      _supla_int64_t value_added, unsigned char value_precision,
      const std::string &unit_before_value, const std::string &unit_after_value,
      unsigned char no_space_before_value, unsigned char no_space_after_value,
      unsigned char keep_history, unsigned _supla_int16_t refresh_interval_ms,
      _supla_int_t default_value_divider, _supla_int_t default_value_multiplier,
      _supla_int64_t default_value_added, unsigned char default_value_precision,
      const std::string &default_unit_before_value,
      const std::string &default_unit_after_value);

  bool get_config(_supla_int_t *value_divider, _supla_int_t *value_multiplier,
                  _supla_int64_t *value_added, unsigned char *value_precision,
                  std::string *unit_before_value, std::string *unit_after_value,
                  unsigned char *no_space_before_value,
                  unsigned char *no_space_after_value,
                  unsigned char *keep_history,
                  unsigned _supla_int16_t *refresh_interval_ms,
                  _supla_int_t *default_value_divider,
                  _supla_int_t *default_value_multiplier,
                  _supla_int64_t *default_value_added,
                  unsigned char *default_value_precision,
                  std::string *default_unit_before_value,
                  std::string *default_unit_after_value);

 public:
  explicit general_purpose_base_config(supla_json_config *root);
  general_purpose_base_config(void);

  double get_value_divider(void);
  double get_value_multiplier(void);
  double get_value_added(void);
  bool keep_history(void);
  std::string get_unit(void);
  unsigned char get_precision(void);
};

#endif /* GENERAL_PURPOSE_BASE_CONFIG_H_ */
