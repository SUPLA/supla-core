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

#ifndef ACTION_OPENCLOSE_H_
#define ACTION_OPENCLOSE_H_

#include "abstract_gate_state_getter.h"
#include "action_executor.h"
#include "asynctask/abstract_asynctask.h"

class supla_action_gate_openclose : public supla_abstract_asynctask {
 private:
  supla_abstract_action_executor *action_executor;
  supla_abstract_gate_state_getter *state_getter;
  int user_id;
  int device_id;
  int channel_id;
  bool open;
  short attempt_count_left;
  unsigned int verification_delay_us;
  void action_init(supla_abstract_action_executor *action_executor,
                   supla_abstract_gate_state_getter *state_getter, int user_id,
                   int device_id, int channel_id,
                   unsigned int verification_delay_us, bool open);

 protected:
  virtual bool _execute(bool *execute_again);
  virtual void task_will_added(void);

 public:
  supla_action_gate_openclose(supla_asynctask_queue *queue,
                              supla_abstract_asynctask_thread_pool *pool,
                              supla_abstract_action_executor *action_executor,
                              supla_abstract_gate_state_getter *state_getter,
                              int user_id, int device_id, int channel_id,
                              unsigned int verification_delay_us, bool open);

  supla_action_gate_openclose(supla_asynctask_queue *queue,
                              supla_abstract_asynctask_thread_pool *pool,
                              short priority, bool release_immediately,
                              supla_abstract_action_executor *action_executor,
                              supla_abstract_gate_state_getter *state_getter,
                              int user_id, int device_id, int channel_id,
                              unsigned int verification_delay_us, bool open);

  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);
};

#endif /*ACTION_OPENCLOSE_H_*/
