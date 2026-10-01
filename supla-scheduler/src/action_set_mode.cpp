/*
 Copyright (C) AC SOFTWARE SP. Z O.O.
 SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "action_set_mode.h"

#include "cJSON.h"
#include "proto.h"

s_worker_action_set_mode::s_worker_action_set_mode(s_abstract_worker *worker,
                                                   bool action_trigger)
    : s_worker_action(worker), action_trigger(action_trigger) {}

bool s_worker_action_set_mode::is_action_allowed(void) {
  int func = worker->get_channel_func();
  return action_trigger ? func == SUPLA_CHANNELFNC_ACTIONTRIGGER
                        : supla_weekly_schedule_is_relay_function(func);
}

bool s_worker_action_set_mode::get_mode(unsigned char *mode) {
  const char *action_param = worker->get_action_param();
  if (!mode || !action_param) {
    return false;
  }

  cJSON *root = cJSON_Parse(action_param);
  if (!root) {
    return false;
  }

  cJSON *item = cJSON_GetObjectItem(root, "mode");
  bool valid = item && cJSON_IsString(item) &&
               supla_action_mode_from_text(
                   action_trigger ? ACTION_SET_AT_PARAMETERS
                                  : ACTION_SET_RELAY_PARAMETERS,
                   item->valuestring, mode);

  cJSON_Delete(root);
  return valid;
}

bool s_worker_action_set_mode::do_action(void) {
  unsigned char mode = 0;
  if (!get_mode(&mode)) {
    return false;
  }

  return action_trigger ? worker->ipcc_action_set_at_parameters(mode)
                        : worker->ipcc_action_set_relay_parameters(mode);
}

bool s_worker_action_set_mode::result_success(int *fail_result_code) {
  unsigned char mode = 0;
  if (!get_mode(&mode)) {
    return false;
  }

  if (action_trigger) {
    TActionTriggerProperties value = {};
    return worker->ipcc_get_action_trigger_value(&value) &&
           value.ButtonMode == mode;
  }

  TRelayChannel_Value value = {};
  return worker->ipcc_get_relay_value(&value) && value.RelayMode == mode;
}

int s_worker_action_set_mode::try_limit(void) { return 2; }

int s_worker_action_set_mode::waiting_time_to_retry(void) { return 30; }

int s_worker_action_set_mode::waiting_time_to_check(void) { return 5; }

REGISTER_ACTION(s_worker_action_set_at_parameters, ACTION_SET_AT_PARAMETERS);
REGISTER_ACTION(s_worker_action_set_relay_parameters,
                ACTION_SET_RELAY_PARAMETERS);
