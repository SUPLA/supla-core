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

#include "value_based_triggers.h"

#include <string>

#include "actions/action_executor.h"
#include "db/db_access_provider.h"
#include "device/channel_property_getter.h"
#include "lck.h"
#include "vbt/value_based_trigger_dao.h"

using std::shared_ptr;
using std::vector;

supla_value_based_triggers::supla_value_based_triggers(supla_user *user) {
  this->user = user;
  this->lck = lck_init();
}

supla_value_based_triggers::~supla_value_based_triggers(void) { lck_free(lck); }

void supla_value_based_triggers::load(void) {
  supla_db_access_provider dba;
  supla_value_based_trigger_dao dao(&dba);

  std::vector<supla_value_based_trigger *> triggers =
      dao.get_triggers(user->getUserID());

  lck_lock(lck);

  for (auto it = this->triggers.begin(); it != this->triggers.end(); ++it) {
    auto nit = triggers.begin();
    while (nit != triggers.end()) {
      if ((*it)->get_id() == (*nit)->get_id()) {
        if (*(*nit) != it->get()) {
          nit = triggers.end();
        }
        break;
      }
      ++nit;
    }

    // Remove if it doesn't exist or has changed
    if (nit == triggers.end()) {
      it = this->triggers.erase(it);
      --it;
    }
  }

  for (auto nit = triggers.begin(); nit != triggers.end(); ++nit) {
    auto it = this->triggers.begin();
    while (it != this->triggers.end()) {
      if ((*it)->get_id() == (*nit)->get_id()) {
        break;
      }
      ++it;
    }

    // Add if not exists
    if (it == this->triggers.end()) {
      this->triggers.push_back(shared_ptr<supla_value_based_trigger>(*nit));
      nit = triggers.erase(nit);
      --nit;
    }
  }

  lck_unlock(lck);

  for (auto nit = triggers.begin(); nit != triggers.end(); ++nit) {
    delete *nit;
  }
}

size_t supla_value_based_triggers::count(void) {
  lck_lock(lck);
  size_t result = triggers.size();
  lck_unlock(lck);
  return result;
}

shared_ptr<supla_value_based_trigger> supla_value_based_triggers::get(int id) {
  shared_ptr<supla_value_based_trigger> result;
  lck_lock(lck);
  for (auto it = triggers.begin(); it != triggers.end(); ++it) {
    if ((*it)->get_id() == id) {
      result = *it;
      break;
    }
  }
  lck_unlock(lck);
  return result;
}

void supla_value_based_triggers::on_channel_value_changed(
    const supla_caller &caller, int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value, supla_channel_extended_value *old_evalue,
    supla_channel_extended_value *new_evalue,
    supla_abstract_action_executor *action_executor,
    supla_abstract_channel_property_getter *property_getter) {
  vector<supla_vbt_condition_result> matches;

  double latitude = 0;
  double longitude = 0;
  std::string timezone = user->get_timezone(&latitude, &longitude);

  lck_lock(lck);
  for (auto it = triggers.begin(); it != triggers.end(); ++it) {
    if ((*it)->get_active_period().is_now_active(timezone.c_str(), latitude,
                                                 longitude)) {
      supla_vbt_condition_result m =
          new_value || old_value
              ? (*it)->are_conditions_met(channel_id, old_value, new_value)
              : (*it)->are_conditions_met(channel_id, old_evalue, new_evalue);

      if (m.are_conditions_met()) {
        m.set_trigger(*it);
        matches.push_back(m);
      }
    }
  }
  lck_unlock(lck);

  // We fire triggers only after leaving the lock.
  for (auto it = matches.begin(); it != matches.end(); ++it) {
    auto replacement_map = it->get_replacement_map();
    it->get_trigger()->fire(caller, user->getUserID(), action_executor,
                            property_getter, &replacement_map);
  }
}

void supla_value_based_triggers::on_channel_value_changed(
    const supla_caller &caller, int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value,
    supla_abstract_action_executor *action_executor,
    supla_abstract_channel_property_getter *property_getter) {
  on_channel_value_changed(caller, channel_id, old_value, new_value, nullptr,
                           nullptr, action_executor, property_getter);
}

void supla_value_based_triggers::on_channel_value_changed(
    const supla_caller &caller, int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value) {
  supla_action_executor exec;
  supla_cahnnel_property_getter property_getter;

  on_channel_value_changed(caller, channel_id, old_value, new_value, &exec,
                           &property_getter);
}

void supla_value_based_triggers::on_channel_value_changed(
    const supla_caller &caller, int channel_id,
    supla_channel_extended_value *old_value,
    supla_channel_extended_value *new_value,
    supla_abstract_action_executor *action_executor,
    supla_abstract_channel_property_getter *property_getter) {
  on_channel_value_changed(caller, channel_id, nullptr, nullptr, old_value,
                           new_value, action_executor, property_getter);
}

void supla_value_based_triggers::on_channel_value_changed(
    const supla_caller &caller, int channel_id,
    supla_channel_extended_value *old_value,
    supla_channel_extended_value *new_value) {
  supla_action_executor exec;
  supla_cahnnel_property_getter property_getter;

  on_channel_value_changed(caller, channel_id, old_value, new_value, &exec,
                           &property_getter);
}
