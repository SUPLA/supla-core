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

#ifndef VBT_SCHEDULER_H_
#define VBT_SCHEDULER_H_

#include <memory>
#include <vector>

#include "actions/abstract_action_executor.h"
#include "device/abstract_channel_property_getter.h"
#include "eh.h"
#include "vbt/value_based_trigger.h"
#include "vbt/vbt_condition_result.h"
#include "vbt_on_change_condition.h"

class supla_vbt_scheduler {
 private:
  static supla_vbt_scheduler *_global_instance;
  std::vector<supla_vbt_condition_result> delayed;
  void *lck;
  void *sthread;
  TEventHandler *eh;
  static void loop(void *scheduler, void *sthread);
  void loop(void *sthread);
  supla_abstract_action_executor *action_executor;
  supla_abstract_channel_property_getter *property_getter;

 public:
  supla_vbt_scheduler(supla_abstract_action_executor *action_executor,
                      supla_abstract_channel_property_getter *property_getter);
  virtual ~supla_vbt_scheduler(void);
  static supla_vbt_scheduler *global_instance(void);
  void erase(const std::shared_ptr<supla_value_based_trigger> &trigger);
  void add_delayed(const std::vector<supla_vbt_condition_result> &delayed);
};

#endif /* VBT_SCHEDULER_H_*/
