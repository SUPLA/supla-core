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

#include "vbt_condition_result.h"

#include "vbt/value_based_trigger.h"

using std::map;
using std::shared_ptr;
using std::string;

supla_vbt_condition_result::supla_vbt_condition_result(bool cnd_met) {
  this->cnd_met = cnd_met;
}

supla_vbt_condition_result::~supla_vbt_condition_result(void) {}

const shared_ptr<supla_value_based_trigger>
    &supla_vbt_condition_result::get_trigger(void) {
  return trigger;
}

void supla_vbt_condition_result::set_trigger(
    const shared_ptr<supla_value_based_trigger> &trigger) {
  this->trigger = trigger;
}

const map<string, string> &supla_vbt_condition_result::get_replacement_map(
    void) {
  return replacement_map;
}

void supla_vbt_condition_result::set_replacement_map(
    const map<string, string> &replacement_map) {
  this->replacement_map = replacement_map;
}

bool supla_vbt_condition_result::are_conditions_met(void) { return cnd_met; }
