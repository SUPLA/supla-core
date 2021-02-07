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

#ifndef ABSTRACT_SCHEDULED_TASK_ACTION_H_
#define ABSTRACT_SCHEDULED_TASK_ACTION_H_

#include <sys/time.h>
#include <list>
#include "abstract_execution_condition.h"

enum scheduled_task_action_state {
  STA_SATE_WAITING,
  STA_SATE_EXECUTING,
  STA_SATE_SUCCESS,
  STA_SATE_FAILURE,
};

class supla_abstract_scheduled_task;
class supla_abstract_scheduled_task_action {
 private:
  void *lck;
  supla_abstract_scheduled_task *task;
  std::list<supla_abstract_execution_condition *> exec_conditions;
  scheduled_task_action_state state;
  long long delay_usec;
  struct timeval conditions_met_at_time;
  struct timeval exec_start_at_time;

 protected:
  friend class supla_abstract_scheduled_task;

  void lock(void);
  void unlock(void);
  void set_task(supla_abstract_scheduled_task *task);
  virtual bool _execute(void) = 0;

 public:
  supla_abstract_scheduled_task_action(void);
  virtual ~supla_abstract_scheduled_task_action(void);
  supla_abstract_scheduled_task *get_task(void);
  void add_execution_condition(supla_abstract_execution_condition *cnd);
  void verify_conditions(void);
  bool is_execution_allowed(void);
  long long time_left_to_execution(void);
  bool is_executed(void);
  scheduled_task_action_state get_state(void);

  long long get_delay_usec(void);
  void set_delay_usec(long long delay_usec);
  void execute(void);
};

#endif /*ABSTRACT_SCHEDULED_TASK_ACTION_H_*/
