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

#ifndef VALUE_BASED_TRIGGER_H_
#define VALUE_BASED_TRIGGER_H_

#include <map>
#include <string>
#include <vector>

#include "actions/abstract_action_executor.h"
#include "actions/action_config.h"
#include "active_period.h"
#include "user/user.h"
#include "vbt/vbt_condition_result.h"
#include "vbt/vbt_on_change_condition.h"

class supla_value_based_trigger {
 private:
  int id;
  int channel_id;
  supla_vbt_on_change_condition on_change_cnd;
  supla_action_config action_config;
  supla_active_period active_period;

  struct timeval first_fire_time;
  struct timeval last_fire_try_time;
  long long min_time_between_firing_usec;
  unsigned int fire_counter;
  unsigned int fire_count_limit;
  int time_window_sec;

 public:
  explicit supla_value_based_trigger(int id, int channel_id,
                                     const supla_action_config &action_config,
                                     const char *conditions,
                                     const supla_active_period &active_period);
  virtual ~supla_value_based_trigger(void);

  int get_id(void);
  int get_channel_id(void);

  int get_time_window_sec(void);
  void set_time_window_sec(int time_window_sec);
  unsigned int get_fire_count_limit(void);
  void set_fire_count_limit(unsigned int fire_count_limit);
  long long get_min_time_between_firing_usec(void);
  void set_min_time_between_firing_usec(long long min_time_between_firing_usec);

  supla_action_config get_action_config(void);
  const supla_vbt_on_change_condition &get_on_change_cnd(void);
  const supla_active_period &get_active_period(void);

  supla_vbt_condition_result are_conditions_met(int channel_id,
                                                supla_vbt_value *old_value,
                                                supla_vbt_value *new_value);

  bool is_now_active(const char *timezone, double latitude, double longitude);

  void fire(const supla_caller &caller, int user_id,
            supla_abstract_action_executor *action_executor,
            supla_abstract_channel_property_getter *property_getter,
            std::map<std::string, std::string> *replacement_map);

  bool equal(const supla_value_based_trigger &trigger) const;

  bool operator==(const supla_value_based_trigger &trigger) const;
  bool operator!=(const supla_value_based_trigger &trigger) const;
  bool operator==(const supla_value_based_trigger *trigger) const;
  bool operator!=(const supla_value_based_trigger *trigger) const;
};

#endif /* VALUE_BASED_TRIGGER_H_ */
