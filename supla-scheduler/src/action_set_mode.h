/*
 Copyright (C) AC SOFTWARE SP. Z O.O.
 SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ACTION_SET_MODE_H_
#define ACTION_SET_MODE_H_

#include "action.h"

class s_worker_action_set_mode : public s_worker_action {
 private:
  bool action_trigger;

 protected:
  virtual bool is_action_allowed(void);
  virtual bool result_success(int *fail_result_code);
  virtual bool do_action(void);

 public:
  s_worker_action_set_mode(s_abstract_worker *worker, bool action_trigger);
  bool get_mode(unsigned char *mode);
  virtual int try_limit(void);
  virtual int waiting_time_to_retry(void);
  virtual int waiting_time_to_check(void);
};

class s_worker_action_set_at_parameters : public s_worker_action_set_mode {
 public:
  explicit s_worker_action_set_at_parameters(s_abstract_worker *worker)
      : s_worker_action_set_mode(worker, true) {}
};

class s_worker_action_set_relay_parameters : public s_worker_action_set_mode {
 public:
  explicit s_worker_action_set_relay_parameters(s_abstract_worker *worker)
      : s_worker_action_set_mode(worker, false) {}
};

#endif /* ACTION_SET_MODE_H_ */
