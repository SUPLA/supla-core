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

#include "value_based_trigger.h"

#include "log.h"
#include "vbt/vbt_on_change_condition.h"

supla_value_based_trigger::supla_value_based_trigger(
    int id, int channel_id, const supla_action_config &action_config,
    const char *conditions, const supla_active_period &active_period) {
  this->id = id;
  this->channel_id = channel_id;
  this->action_config = action_config;
  this->first_fire_time = {};
  this->last_fire_try_time = {};
  this->min_time_between_firing_usec = 100000;
  this->fire_counter = 0;
  this->fire_count_limit = 5;
  this->time_window_sec = 5;
  this->active_period = active_period;

  cJSON *json = nullptr;

  if (conditions && conditions[0] != 0) {
    json = cJSON_Parse(conditions);

    if (json) {
      on_change_cnd.apply_json_config(json);
      cJSON_Delete(json);
    }
  }
}

supla_value_based_trigger::~supla_value_based_trigger(void) {}

supla_vbt_condition_result supla_value_based_trigger::are_conditions_met(
    int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value) {
  bool cnd_met = channel_id == this->channel_id &&
                 on_change_cnd.is_condition_met(old_value, new_value);
  supla_vbt_condition_result result(cnd_met);
  result.set_replacement_map(new_value->get_replacement_map());
  return result;
}

supla_vbt_condition_result supla_value_based_trigger::are_conditions_met(
    int channel_id, supla_channel_extended_value *old_value,
    supla_channel_extended_value *new_value) {
  bool cnd_met = channel_id == this->channel_id &&
                 on_change_cnd.is_condition_met(old_value, new_value);
  supla_vbt_condition_result result(cnd_met);
  result.set_replacement_map(new_value->get_replacement_map());
  return result;
}

void supla_value_based_trigger::fire(
    const supla_caller &caller, int user_id,
    supla_abstract_action_executor *action_executor,
    supla_abstract_channel_property_getter *property_getter,
    std::map<std::string, std::string> *replacement_map) {
  struct timeval now = {};
  gettimeofday(&now, nullptr);

  if (min_time_between_firing_usec) {
    if (((now.tv_sec * 1000000LL + now.tv_usec) -
         (last_fire_try_time.tv_sec * 1000000LL +
          last_fire_try_time.tv_usec)) <= min_time_between_firing_usec) {
      last_fire_try_time = now;
      supla_log(LOG_WARNING,
                "The trigger was fired too fast. VbtId: %i, UserId: %i",
                get_id(), user_id);
      return;
    }

    last_fire_try_time = now;
  }

  if (fire_count_limit) {
    if (first_fire_time.tv_sec == 0) {
      first_fire_time = now;
    } else if (now.tv_sec - first_fire_time.tv_sec >= time_window_sec) {
      fire_counter = 0;
      first_fire_time = {};
    }

    fire_counter++;
    if (fire_counter > fire_count_limit) {
      if (fire_counter == fire_count_limit + 1) {
        supla_log(LOG_WARNING,
                  "The trigger was fired too fast. VbtId: %i, UserId: %i",
                  get_id(), user_id);
        supla_log(LOG_WARNING,
                  "Exceeded %i value-based trigger firings in %i seconds. For "
                  "the next %i seconds firings will be ignored. VbtId: %i, "
                  "UserId: %i",
                  fire_count_limit, time_window_sec, time_window_sec, get_id(),
                  user_id);
        first_fire_time = now;  // Extends reset for the same amount of time
      }
      return;
    }
  }

  action_executor->execute_action(caller, user_id, &action_config,
                                  property_getter, replacement_map);
}

int supla_value_based_trigger::get_id(void) { return id; }

int supla_value_based_trigger::get_channel_id(void) { return channel_id; }

int supla_value_based_trigger::get_time_window_sec(void) {
  return time_window_sec;
}

void supla_value_based_trigger::set_time_window_sec(int time_window_sec) {
  this->time_window_sec = time_window_sec;
}

unsigned int supla_value_based_trigger::get_fire_count_limit(void) {
  return fire_count_limit;
}

void supla_value_based_trigger::set_fire_count_limit(
    unsigned int fire_count_limit) {
  this->fire_count_limit = fire_count_limit;
}

long long supla_value_based_trigger::get_min_time_between_firing_usec(void) {
  return min_time_between_firing_usec;
}

void supla_value_based_trigger::set_min_time_between_firing_usec(
    long long min_time_between_firing_usec) {
  this->min_time_between_firing_usec = min_time_between_firing_usec;
}

supla_action_config supla_value_based_trigger::get_action_config(void) {
  return action_config;
}

const supla_vbt_on_change_condition &
supla_value_based_trigger::get_on_change_cnd(void) {
  return on_change_cnd;
}

const supla_active_period &supla_value_based_trigger::get_active_period(void) {
  return active_period;
}

bool supla_value_based_trigger::equal(
    const supla_value_based_trigger &trigger) const {
  return id == trigger.id && channel_id == trigger.channel_id &&
         on_change_cnd == trigger.on_change_cnd &&
         action_config == trigger.action_config &&
         active_period == trigger.active_period;
}

bool supla_value_based_trigger::operator==(
    const supla_value_based_trigger &trigger) const {
  return equal(trigger);
}

bool supla_value_based_trigger::operator!=(
    const supla_value_based_trigger &trigger) const {
  return !equal(trigger);
}

bool supla_value_based_trigger::operator==(
    const supla_value_based_trigger *trigger) const {
  return *this == *trigger;
}

bool supla_value_based_trigger::operator!=(
    const supla_value_based_trigger *trigger) const {
  return *this != *trigger;
}
