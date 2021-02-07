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

#include "abstract_scheduled_task_action.h"
#include "abstract_scheduled_task.h"
#include "lck.h"

supla_abstract_scheduled_task_action::supla_abstract_scheduled_task_action(
    void) {
  lck = lck_init();
  task = NULL;
  state = STA_SATE_WAITING;
  gettimeofday(&conditions_met_at_time, NULL);
  delay_usec = 0;
}

supla_abstract_scheduled_task_action::~supla_abstract_scheduled_task_action(
    void) {
  lck_free(lck);
}

void supla_abstract_scheduled_task_action::lock(void) { lck_lock(lck); }

void supla_abstract_scheduled_task_action::unlock(void) { lck_unlock(lck); }

void supla_abstract_scheduled_task_action::set_task(
    supla_abstract_scheduled_task *task) {
  lock();
  this->task = task;
  unlock();
}

supla_abstract_scheduled_task *supla_abstract_scheduled_task_action::get_task(
    void) {
  supla_abstract_scheduled_task *result = NULL;
  lock();
  result = task;
  unlock();
  return result;
}

void supla_abstract_scheduled_task_action::add_execution_condition(
    supla_abstract_execution_condition *cnd) {
  lock();
  exec_conditions.push_back(cnd);
  conditions_met_at_time.tv_sec = 0;
  conditions_met_at_time.tv_usec = 0;
  unlock();
}

void supla_abstract_scheduled_task_action::verify_conditions(void) {
  lock();
  if (!conditions_met_at_time.tv_sec && !conditions_met_at_time.tv_usec) {
    bool conditions_met = true;
    for (std::list<supla_abstract_execution_condition *>::iterator it =
             exec_conditions.begin();
         it != exec_conditions.end(); ++it) {
      if (!(*it)->is_execution_allowed()) {
        conditions_met = false;
        break;
      }
    }

    if (conditions_met) {
      gettimeofday(&conditions_met_at_time, NULL);
    }
  }
  unlock();
}

bool supla_abstract_scheduled_task_action::is_execution_allowed(void) {
  verify_conditions();

  lock();
  bool result = conditions_met_at_time.tv_sec || conditions_met_at_time.tv_usec;
  unlock();

  return result;
}

long long supla_abstract_scheduled_task_action::time_left_to_execution(void) {
  long long result = 0x7FFFFFFFFFFFFFFF;
  lock();

  if (is_execution_allowed()) {
    struct timeval now;
    gettimeofday(&now, NULL);
    long long diff = now.tv_sec * (long long)1000000 + now.tv_usec;
    diff -= conditions_met_at_time.tv_sec * (long long)1000000 +
            conditions_met_at_time.tv_usec;

    result = delay_usec - diff;
  }

  unlock();
  return result;
}

bool supla_abstract_scheduled_task_action::is_executed(void) {
  lock();
  bool result = state == STA_SATE_SUCCESS || state == STA_SATE_FAILURE;
  unlock();

  return result;
}

scheduled_task_action_state supla_abstract_scheduled_task_action::get_state(
    void) {
  lock();
  scheduled_task_action_state result = state;
  unlock();

  return result;
}

long long supla_abstract_scheduled_task_action::get_delay_usec(void) {
  lock();
  long long result = delay_usec;
  unlock();
  return result;
}

void supla_abstract_scheduled_task_action::set_delay_usec(
    long long delay_usec) {
  bool changed = false;
  lock();
  if (this->delay_usec != delay_usec) {
    this->delay_usec = delay_usec;
    changed = true;
  }
  unlock();

  if (changed) {
    supla_abstract_scheduled_task *task = get_task();
    if (task) {
      task->on_task_action_delay_changed(this);
    }
  }
}

void supla_abstract_scheduled_task_action::execute(void) {
  lock();
  bool exec_allowed = state == STA_SATE_WAITING && is_execution_allowed();
  if (exec_allowed) {
    state = STA_SATE_EXECUTING;
    gettimeofday(&exec_start_at_time, NULL);
  }
  unlock();

  if (!exec_allowed) {
    return;
  }

  bool exec_result = _execute();

  lock();
  state = exec_result ? STA_SATE_SUCCESS : STA_SATE_FAILURE;
  unlock();

  supla_abstract_scheduled_task *task = get_task();
  if (task) {
    task->verify_conditions(this);
  }
}
