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

#include <memory>
#include <string>

#include "srpc/multipart_call.h"
#include "vbt/value_based_trigger.h"

using std::map;
using std::shared_ptr;
using std::string;

supla_vbt_condition_result::supla_vbt_condition_result(
    bool cnd_met, _supla_int64_t milliseconds_left) {
  this->cnd_met = cnd_met;
  this->milliseconds_left = milliseconds_left;
}

supla_vbt_condition_result::~supla_vbt_condition_result(void) {}

const shared_ptr<supla_value_based_trigger> &
supla_vbt_condition_result::get_trigger(void) const {
  return trigger;
}

void supla_vbt_condition_result::set_trigger(
    const shared_ptr<supla_value_based_trigger> &trigger) {
  this->trigger = trigger;
}

const nlohmann::json &supla_vbt_condition_result::get_template_data(
    void) const {
  return template_data;
}

void supla_vbt_condition_result::set_template_data(
    const nlohmann::json &template_data) {
  this->template_data = template_data;
}

void supla_vbt_condition_result::set_caller(const supla_caller &caller) {
  this->caller = caller;
}

supla_caller supla_vbt_condition_result::get_caller(void) const {
  return caller;
}

void supla_vbt_condition_result::set_user(supla_user *user) {
  this->user = user;
}

supla_user *supla_vbt_condition_result::get_user(void) const { return user; }

bool supla_vbt_condition_result::is_condition_met(void) { return cnd_met; }

bool supla_vbt_condition_result::is_condition_met(
    _supla_int64_t *milliseconds_left) {
  *milliseconds_left = this->milliseconds_left;
  return cnd_met;
}
