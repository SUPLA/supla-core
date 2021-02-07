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

#include "abstract_scheduled_task.h"
#include "lck.h"

supla_abstract_scheduled_task::supla_abstract_scheduled_task(void) {
  lck = lck_init();
}

supla_abstract_scheduled_task::~supla_abstract_scheduled_task(void) {
  cleanup();
  lck_free(lck);
}

void supla_abstract_scheduled_task::lock(void) { lck_lock(lck); }

void supla_abstract_scheduled_task::unlock(void) { lck_unlock(lck); }

void supla_abstract_scheduled_task::cleanup(void) {
  lock();
  for (std::list<supla_abstract_scheduled_task_action *>::iterator it =
           actions.begin();
       it != actions.end(); ++it) {
    delete *it;
  }

  actions.clear();
  unlock();
}

bool supla_abstract_scheduled_task::action_exists(
    supla_abstract_scheduled_task_action *action) {
  for (std::list<supla_abstract_scheduled_task_action *>::iterator it =
           actions.begin();
       it != actions.end(); ++it) {
    if (*it == action) {
      return true;
    }
  }

  return false;
}

void supla_abstract_scheduled_task::add_action(
    supla_abstract_scheduled_task_action *action) {
  bool added = false;
  lock();
  if (!action_exists(action)) {
    actions.push_back(action);
    action->set_task(this);
    added = true;
  }
  unlock();

  if (added) {
    recalculate();
  }
}

void supla_abstract_scheduled_task::on_task_action_delay_changed(
    supla_abstract_scheduled_task_action *sender) {
  recalculate();
}

void supla_abstract_scheduled_task::verify_conditions(
    supla_abstract_scheduled_task_action *sender) {
  recalculate();
}

void supla_abstract_scheduled_task::recalculate(void) {}
