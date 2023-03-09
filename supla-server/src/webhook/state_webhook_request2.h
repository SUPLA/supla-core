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
#ifndef WEBHOOK_STATE_WEBHOOK_REQUEST2_H_
#define WEBHOOK_STATE_WEBHOOK_REQUEST2_H_

#include <string>

#include "http/asynctask_http_request.h"
#include "webhook/state_webhook_credentials2.h"

class supla_state_webhook_request2 : public supla_asynctask_http_request {
 private:
  int actions;
  supla_state_webhook_credentials2 *credentials;

 protected:
  virtual bool make_request(supla_abstract_curl_adapter *curl_adapter);
  virtual std::string get_name(void);

 public:
  supla_state_webhook_request2(
      const supla_caller &caller, int user_id, int device_id, int channel_id,
      int actions, supla_asynctask_queue *queue,
      supla_abstract_asynctask_thread_pool *pool,
      supla_abstract_channel_property_getter *property_getter,
      supla_state_webhook_credentials2 *credentials);

  virtual ~supla_state_webhook_request2(void);

  bool is_any_action_set(void);

  static bool is_caller_allowed(const supla_caller &caller);
  static bool is_function_allowed(int func,
                                  supla_state_webhook_credentials2 *credentials,
                                  int *delay_time_usec);
  static void new_request(const supla_caller &caller, supla_user *user,
                          int device_id, int channel_id, int actions);
};

#endif /* WEBHOOK_STATE_WEBHOOK_REQUEST2_H_ */
