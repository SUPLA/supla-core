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

#include "action_gate_openclose.h"

#include <assert.h>

#include "action_gate_openclose_search_condition.h"
#include "actions/action_executor.h"
#include "asynctask/asynctask_default_thread_pool.h"
#include "asynctask/asynctask_queue.h"
#include "device/channel_property_getter.h"
#include "device/value/channel_gate_value.h"
#include "jsonconfig/channel/channel_json_config_getter.h"
#include "jsonconfig/channel/controlling_the_gate_config.h"
#include "log.h"

#define VERIFICATION_DELAY_US 60000000

supla_action_gate_openclose::supla_action_gate_openclose(
    const supla_caller &caller, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_action_executor *action_executor,
    supla_abstract_channel_property_getter *property_getter,
    abstract_json_config_getter *json_config_getter, int user_id, int device_id,
    int channel_id, unsigned int verification_delay_us, bool open)
    : supla_abstract_asynctask(queue, pool) {
  assert(action_executor);
  this->action_executor = action_executor;
  this->caller = caller;
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;
  this->open = open;
  this->property_getter = property_getter;
  this->attempt_count_left = 0;
  this->verification_delay_us = verification_delay_us;

  supla_json_config *json_config = NULL;
  if (json_config_getter) {
    json_config =
        json_config_getter->get_config(user_id, device_id, channel_id);

    delete json_config_getter;
    json_config_getter = NULL;
  }

  controlling_the_gate_config *json_gconfig =
      new controlling_the_gate_config(json_config);

  if (json_gconfig) {
    this->attempt_count_left =
        open ? json_gconfig->get_number_of_attempts_to_open()
             : json_gconfig->get_number_of_attempts_to_close();
    delete json_gconfig;
    json_gconfig = NULL;
  }

  if (json_config) {
    delete json_config;
    json_config = NULL;
  }

  action_executor->set_channel_id(user_id, device_id, channel_id);
}

supla_action_gate_openclose::~supla_action_gate_openclose(void) {
  if (property_getter) {
    delete property_getter;
  }

  if (action_executor) {
    delete action_executor;
  }
}

const supla_caller &supla_action_gate_openclose::get_caller(void) const {
  return caller;
}

int supla_action_gate_openclose::get_user_id(void) { return user_id; }

int supla_action_gate_openclose::get_device_id(void) { return device_id; }

int supla_action_gate_openclose::get_channel_id(void) { return channel_id; }

bool supla_action_gate_openclose::action_open(void) { return open; }

bool supla_action_gate_openclose::get_closing_state(bool *is_closed) {
  if (!is_closed) {
    return false;
  }

  _gate_sensor_level_enum opening_sensor_level = gsl_unknown;

  supla_abstract_channel_value *value = property_getter->get_value(
      get_user_id(), get_device_id(), get_channel_id());
  if (value) {
    supla_channel_gate_value *gate_value =
        dynamic_cast<supla_channel_gate_value *>(value);
    if (gate_value) {
      opening_sensor_level = gate_value->get_opening_sensor_level();
    }
    delete value;
    value = NULL;
  }

  if (opening_sensor_level != gsl_unknown) {
    *is_closed = opening_sensor_level == gsl_closed;
  }

  return opening_sensor_level != gsl_unknown;
}

bool supla_action_gate_openclose::_execute(
    bool *execute_again, supla_asynctask_thread_bucket *bucket) {
  if (!property_getter || !action_executor) {
    return false;
  }

  bool is_closed = open;
  attempt_count_left--;

  if (!get_closing_state(&is_closed)) {
    return false;
  }

  if (attempt_count_left < 0 || open != is_closed) {
    return open != is_closed;
  }

  action_executor->open_close_without_canceling_tasks();
  *execute_again = true;
  set_delay_usec(verification_delay_us);

  return false;
}

void supla_action_gate_openclose::on_timeout(
    unsigned long long timeout_usec, unsigned long long usec_after_timeout) {
  supla_log(LOG_WARNING,
            "Gate open/close - aciton execution timeout. ChannelId: %i, "
            "TimeoutUSec: %llu+%llu",
            get_channel_id(), timeout_usec, usec_after_timeout);
}

// static
void supla_action_gate_openclose::cancel_tasks(int user_id, int device_id,
                                               int channel_id) {
  supla_action_gate_openclose_search_condition cnd(NULL, user_id, device_id,
                                                   channel_id, false, false);

  supla_asynctask_queue::global_instance()->cancel_tasks(&cnd);
}

// static
void supla_action_gate_openclose::cancel_tasks(int user_id, int device_id,
                                               int channel_id,
                                               bool action_open) {
  supla_action_gate_openclose_search_condition cnd(
      NULL, user_id, device_id, channel_id, true, action_open);

  supla_asynctask_queue::global_instance()->cancel_tasks(&cnd);
}

// static
void supla_action_gate_openclose::open_close(const supla_caller &caller,
                                             int user_id, int device_id,
                                             int channel_id, bool open) {
  {
    supla_action_gate_openclose_search_condition cnd(NULL, user_id, device_id,
                                                     channel_id, true, open);

    if (supla_asynctask_queue::global_instance()->task_exists(&cnd)) {
      return;
    }
  }

  cancel_tasks(user_id, device_id, channel_id);

  supla_action_executor *action_executor = new supla_action_executor();
  supla_channel_property_getter *property_getter =
      new supla_channel_property_getter();
  channel_json_config_getter *config_getter = new channel_json_config_getter();

  supla_action_gate_openclose *task = new supla_action_gate_openclose(
      caller, supla_asynctask_queue::global_instance(),
      supla_asynctask_default_thread_pool::global_instance(), action_executor,
      property_getter, config_getter, user_id, device_id, channel_id,
      VERIFICATION_DELAY_US, open);
  task->start();
}
