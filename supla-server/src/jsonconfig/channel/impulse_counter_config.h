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

#ifndef IMPULSECOUNTERCONFIG_H_
#define IMPULSECOUNTERCONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class impulse_counter_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  double get_double_value(const char *key, double min, double max,
                          double multiplier);
  bool get_custom_unit(cJSON *user_root, char custom_unit[9],
                       int use_defaults_for_function);
  bool get_currency(cJSON *user_root, std::string *currency);

 protected:
  std::map<unsigned _supla_int16_t, std::string> get_field_map(void);

 public:
  explicit impulse_counter_config(supla_json_config *root);
  impulse_counter_config(void);
  bool should_be_added_to_history(void);
  double get_initial_value(void);
  unsigned int get_impulses_per_unit(void);
  void add_initial_value(unsigned _supla_int64_t *counter);
  void get_custom_unit(char custom_unit[9], int use_defaults_for_function);
  int get_price_per_unit(void);
  std::string get_currency(void);

  virtual void merge(supla_json_config *_dst);
  void set_config(TChannelConfig_ImpulseCounter *config);
  bool get_config(TChannelConfig_ImpulseCounter *config);
};

#endif /* IMPULSECOUNTERCONFIG_H_ */
