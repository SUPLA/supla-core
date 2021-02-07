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

#include <list>
#include "abstract_scheduled_task_action.h"

#ifndef ABSTRACT_SCHEDULED_TASK_H_
#define ABSTRACT_SCHEDULED_TASK_H_

class supla_abstract_scheduled_task {
 private:
  void *lck;
  std::list<supla_abstract_scheduled_task_action *> actions;
  void cleanup(void);
  bool action_exists(supla_abstract_scheduled_task_action *action);

 protected:
  friend class supla_abstract_scheduled_task_action;

  void lock(void);
  void unlock(void);
  void recalculate(void);
  void verify_conditions(supla_abstract_scheduled_task_action *sender);
  void on_task_action_delay_changed(
      supla_abstract_scheduled_task_action *sender);

 public:
  supla_abstract_scheduled_task(void);
  virtual ~supla_abstract_scheduled_task(void);
  void add_action(supla_abstract_scheduled_task_action *action);
};

#endif /*ABSTRACT_SCHEDULED_TASK_H_*/
