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

#include "google_home_sync_request.h"

#include "google/google_home_client.h"
#include "google/google_home_sync_search_condition.h"
#include "google/google_home_sync_thread_pool.h"
#include "svrcfg.h"
#include "user/user.h"

using std::string;

supla_google_home_sync_request::supla_google_home_sync_request(
    int user_id, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool,
    supla_google_home_credentials *credentials)
    : supla_asynctask_http_request(supla_caller(), user_id, 0, 0, queue, pool,
                                   nullptr) {
  this->credentials = credentials;

  set_delay_usec(10000000);  // 10 sec.
  set_timeout(scfg_int(CFG_GOOGLE_HOME_SYNCREQUEST_TIMEOUT) * 1000);
}

unsigned long long
supla_google_home_sync_request::get_cfg_long_request_time_usec(void) {
  return 10000000;  // 10 sec.
}

string supla_google_home_sync_request::get_name(void) {
  return "GoogleHome Sync Request";
}

bool supla_google_home_sync_request::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (credentials->is_access_token_exists()) {
    supla_google_home_client client(0, curl_adapter, credentials);
    return client.sync();
  }

  return false;
}

// static
void supla_google_home_sync_request::new_request(supla_user *user) {
  if (!user || !user->googleHomeCredentials() ||
      !user->googleHomeCredentials()->is_access_token_exists()) {
    return;
  }

  bool exists = false;
  supla_google_home_sync_search_condition cnd(user->getUserID());
  supla_asynctask_queue::global_instance()->access_task(
      &cnd,
      [&exists](supla_abstract_asynctask *task) -> void { exists = true; });

  if (!exists) {
    supla_google_home_sync_request *request =
        new supla_google_home_sync_request(
            user->getUserID(), supla_asynctask_queue::global_instance(),
            supla_google_home_sync_thread_pool::global_instance(),
            user->googleHomeCredentials());

    request->start();
  }
}
