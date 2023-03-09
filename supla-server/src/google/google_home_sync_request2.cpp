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

#include "google/google_home_sync_request2.h"

#include "google/google_home_client2.h"
#include "google/google_home_sync_search_condition.h"
#include "http/asynctask_http_thread_pool.h"
#include "svrcfg.h"
#include "user/user.h"

using std::string;

supla_google_home_sync_request2::supla_google_home_sync_request2(
    const supla_caller &caller, int user_id, event_type et,
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_google_home_credentials2 *credentials)
    : supla_asynctask_http_request(caller, user_id, 0, 0, et, queue, pool,
                                   nullptr) {
  this->credentials = credentials;

  set_delay_usec(15000000);  // 15 sec.
  set_timeout(scfg_int(CFG_GOOGLE_HOME_SYNCREQUEST_TIMEOUT) * 1000);
}

string supla_google_home_sync_request2::get_name(void) {
  return "GoogleHome Sync Request";
}

bool supla_google_home_sync_request2::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (credentials->is_access_token_exists()) {
    supla_google_home_client2 client(0, curl_adapter, credentials);
    return client.sync();
  }

  return false;
}

// static
bool supla_google_home_sync_request2::is_event_type_allowed(event_type et) {
  switch (et) {
    case ET_CHANNELS_ADDED:
    case ET_DEVICE_DELETED:
    case ET_USER_RECONNECT:
    case ET_GOOGLE_HOME_SYNC_NEEDED:
      return true;
    default:
      return false;
  }
  return false;
}

// static
void supla_google_home_sync_request2::new_request(const supla_caller &caller,
                                                  supla_user *user,
                                                  event_type et) {
  if (!user || !is_event_type_allowed(et) || !user->googleHomeCredentials() ||
      !user->googleHomeCredentials()->isAccessTokenExists()) {
    return;
  }

  bool exists = false;
  supla_google_home_sync_search_condition cnd(user->getUserID());
  supla_asynctask_queue::global_instance()->access_task(
      &cnd,
      [&exists](supla_abstract_asynctask *task) -> void { exists = true; });

  if (!exists) {
    supla_google_home_sync_request2 *request =
        new supla_google_home_sync_request2(
            caller, user->getUserID(), et,
            supla_asynctask_queue::global_instance(),
            supla_asynctask_http_thread_pool::global_instance(),
            /*user->googleHomeCredentials()*/ nullptr);

    request->start();
  }
}
