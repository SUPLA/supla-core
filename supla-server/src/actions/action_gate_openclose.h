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

#include "actions/action_executor.h"
#include "asynctask/abstract_asynctask.h"
#include "caller.h"
#include "device/abstract_channel_property_getter.h"
#include "jsonconfig/channel/abstract_channel_json_config_getter.h"

class supla_action_gate_openclose : public supla_abstract_asynctask {
 private:
  supla_abstract_action_executor *action_executor;
  supla_abstract_channel_property_getter *property_getter;
  supla_caller caller;
  int user_id;
  int device_id;
  int channel_id;
  bool open;
  short attempt_count_left;
  unsigned int verification_delay_us;

  bool get_closing_state(bool *is_closed);

 protected:
  virtual bool _execute(bool *execute_again,
                        supla_asynctask_thread_bucket *bucket);
  virtual void on_timeout(unsigned long long timeout_usec,
                          unsigned long long usec_after_timeout);

 public:
  supla_action_gate_openclose(
      const supla_caller &caller, supla_asynctask_queue *queue,
      supla_abstract_asynctask_thread_pool *pool,
      supla_abstract_action_executor *action_executor,
      supla_abstract_channel_property_getter *property_getter,
      abstract_channel_json_config_getter *json_config_getter, int user_id,
      int device_id, int channel_id, unsigned int verification_delay_us,
      bool open);
  virtual ~supla_action_gate_openclose(void);

  const supla_caller &get_caller(void) const;
  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);
  bool action_open(void);

  static void cancel_tasks(int user_id, int device_id, int channel_id);
  static void cancel_tasks(int user_id, int device_id, int channel_id,
                           bool action_open);
  static void open_close(const supla_caller &caller, int user_id, int device_id,
                         int channel_id, bool open);
};

#endif /*ACTION_OPENCLOSE_H_*/
