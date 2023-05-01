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

#include "actions/action_executor.h"
#include "lck.h"
#include "push/pn_dispatcher.h"

using std::shared_ptr;
using std::vector;

supla_value_based_triggers::supla_value_based_triggers(supla_user *user) {
  this->user = user;
  this->lck = lck_init();
}

supla_value_based_triggers::~supla_value_based_triggers(void) { lck_free(lck); }

void supla_value_based_triggers::load(void) {}

void supla_value_based_triggers::on_trigger_removed(int id) {}

void supla_value_based_triggers::on_trigger_changed(int id) {}

void supla_value_based_triggers::on_trigger_added(int id) {}

void supla_value_based_triggers::on_channel_value_changed(
    int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value,
    supla_abstract_action_executor *action_executor,
    supla_pn_dispatcher *pn_dispatcher) {
  vector<shared_ptr<supla_value_based_trigger>> matched_triggers;

  lck_lock(lck);
  for (auto it = triggers.rbegin(); it != triggers.rend(); ++it) {
    if ((*it)->are_conditions_met(channel_id, old_value, new_value)) {
      matched_triggers.push_back(*it);
      break;
    }
  }
  lck_unlock(lck);

  for (auto it = matched_triggers.rbegin(); it != matched_triggers.rend();
       ++it) {
    (*it)->fire(action_executor, pn_dispatcher);
  }
}

void supla_value_based_triggers::on_channel_value_changed(
    int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value) {
  supla_action_executor exec;
  supla_pn_dispatcher pn_dispatcher;

  on_channel_value_changed(channel_id, old_value, new_value, &exec,
                           &pn_dispatcher);
}
