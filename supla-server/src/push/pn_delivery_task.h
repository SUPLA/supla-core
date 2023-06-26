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

#ifndef PUSH_NOTIFICATION_DELIVERY_TASK_H_
#define PUSH_NOTIFICATION_DELIVERY_TASK_H_

#include <map>
#include <string>

#include "db/db_access_provider.h"
#include "http/asynctask_http_request.h"
#include "push/pn_gateway_access_token_provider.h"
#include "push/pn_recipient_dao.h"
#include "push/push_notification.h"

class supla_pn_delivery_task : public supla_asynctask_http_request {
 private:
  supla_push_notification *push;
  supla_pn_gateway_access_token_provider *token_provider;
  supla_db_access_provider *dba;
  supla_pn_recipient_dao *recipient_dao;
  void load_content(void);

 protected:
  virtual bool make_request(supla_abstract_curl_adapter *curl_adapter);
  virtual std::string get_name(void);
  virtual long long get_cfg_delay_warning_time_usec(void);
  virtual unsigned long long get_cfg_long_request_time_usec(void);

 public:
  supla_pn_delivery_task(
      int user_id, supla_asynctask_queue *queue,
      supla_abstract_asynctask_thread_pool *pool, supla_push_notification *push,
      supla_pn_gateway_access_token_provider *token_provider);
  virtual ~supla_pn_delivery_task(void);

  static void start_delivering(int user_id, supla_push_notification *push);
  static void start_delivering(
      int user_id, int push_notification_id,
      const std::map<std::string, std::string> *replacement_map);
  static bool start_delivering(int user_id, const char *json);
};

#endif /* PUSH_NOTIFICATION_DELIVERY_TASK_H_ */
