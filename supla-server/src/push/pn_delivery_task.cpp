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

#include "pn_delivery_task.h"

#include "asynctask/asynctask_queue.h"
#include "db/db_access_provider.h"
#include "push/apns_client.h"
#include "push/fcm_client.h"
#include "push/pn_delivery_task_thread_pool.h"
#include "push/pn_recipient_dao.h"
#include "svrcfg.h"

using std::string;

supla_pn_delivery_task::supla_pn_delivery_task(
    int user_id, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool, supla_push_notification *push,
    supla_pn_gateway_access_token_provider *token_provider)
    : supla_asynctask_http_request(supla_caller(), user_id, 0, 0, queue, pool,
                                   nullptr) {
  this->push = push;
  this->token_provider = token_provider;
  set_timeout(scfg_int(CFG_PUSH_REQUEST_TIMEOUT) * 1000);
}

supla_pn_delivery_task::~supla_pn_delivery_task(void) {
  if (push) {
    delete push;
  }
}

long long supla_pn_delivery_task::get_cfg_delay_warning_time_usec(void) {
  return scfg_int(CFG_PUSH_DELAY_WARNING_TIME) * 1000;
}

unsigned long long supla_pn_delivery_task::get_cfg_long_request_time_usec(
    void) {
  return scfg_int(CFG_PUSH_LONG_REQUEST_TIME) * 1000;
}

string supla_pn_delivery_task::get_name(void) { return "Push delivery task"; }

bool supla_pn_delivery_task::will_use_database(void) { return true; }

bool supla_pn_delivery_task::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  bool fcm_recipients = false;
  bool fcm_result = false;

  bool apns_recipients = false;
  bool apns_result = false;

  if (push->get_recipients().count(platform_android) > 0) {
    supla_fcm_client client(curl_adapter, token_provider, push);

    fcm_recipients = true;
    fcm_result = client.send();
  }

  if (push->get_recipients().count(platform_ios) > 0) {
    supla_apns_client client(curl_adapter, token_provider, push);

    apns_recipients = true;
    apns_result = client.send();
  }

  supla_db_access_provider *dba = nullptr;
  supla_pn_recipient_dao *dao = nullptr;

  for (size_t a = 0; a < push->get_recipients().total_count(); a++) {
    supla_pn_recipient *recipient = push->get_recipients().get(a);
    if (!recipient->is_exists()) {
      if (!dba) {
        dba = new supla_db_access_provider();
      }
      if (!dao) {
        dao = new supla_pn_recipient_dao(dba);
      }

      dao->remove(get_user_id(), recipient);
    }
  }

  if (dba) {
    delete dba;
  }

  if (dao) {
    delete dao;
  }

  return (fcm_recipients || apns_recipients) &&
         (!fcm_recipients || fcm_result) && (!apns_recipients || apns_result);
}

// static
void supla_pn_delivery_task::start_delivering(int user_id,
                                              supla_push_notification *push) {
  supla_pn_delivery_task *task = new supla_pn_delivery_task(
      user_id, supla_asynctask_queue::global_instance(),
      supla_pn_delivery_task_thread_pool::global_instance(), push,
      supla_pn_gateway_access_token_provider::global_instance());
  task->start();
}
