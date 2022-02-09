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

#include <channeljsonconfig/channel_json_config.h>

#include <string>

#include "proto.h"

class impulse_counter_config : public channel_json_config {
 private:
  static const char initial_value_key[];
  static const char impulses_per_unit_key[];
  static const char add_to_history_key[];

  double get_double_value(const char *key, double max);

 protected:
 public:
  explicit impulse_counter_config(channel_json_config *root);
  impulse_counter_config(void);
  bool should_be_added_to_history(void);
  double get_initial_value(void);
  unsigned int get_impulses_per_unit(void);
  void add_initial_value(TDS_ImpulseCounter_Value *value);
};

#endif /* IMPULSECOUNTERCONFIG_H_ */
