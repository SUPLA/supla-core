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

#include "alexa_discover_request.h"

#include "amazon/alexa_client.h"
#include "amazon/alexa_discover_payload_obtainer.h"
#include "amazon/alexa_discover_request_search_condition.h"
#include "asynctask/voice_assistant_sync_thread_pool.h"
#include "svrcfg.h"
#include "user/user.h"

using std::string;

supla_alexa_discover_request::supla_alexa_discover_request(
    int user_id, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool,
    supla_amazon_alexa_credentials *credentials,
    supla_alexa_discover_payload_obtainer *obtainer)
    : supla_alexa_request(supla_caller(), user_id, 0, 0, queue, pool, nullptr,
                          credentials) {
  this->obtainer = obtainer;

  set_delay_usec(10000000);  // 10 sec.
  set_timeout(scfg_int(CFG_ALEXA_DISCOVER_REQUEST_TIMEOUT) * 1000);
}

supla_alexa_discover_request::~supla_alexa_discover_request(void) {
  if (obtainer) {
    delete obtainer;
  }
}

bool supla_alexa_discover_request::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (!obtainer || !get_credentials()->is_access_token_exists()) {
    return false;
  }

  string payload = obtainer->obtain(get_user_id(), curl_adapter);
  if (payload.empty()) {
    return false;
  }

  supla_alexa_client client(get_channel_id(), curl_adapter, get_credentials(),
                            "", "", "");

  return client.send_payload(payload);
}

string supla_alexa_discover_request::get_name(void) {
  return "Alexa Discover Request";
}

// static
void supla_alexa_discover_request::new_request(supla_user *user) {
  bool exists = false;
  supla_alexa_discover_request_search_condition cnd(user->getUserID());
  supla_asynctask_queue::global_instance()->access_task(
      &cnd,
      [&exists](supla_abstract_asynctask *task) -> void { exists = true; });

  if (!exists) {
    supla_db_access_provider *dba = new supla_db_access_provider();
    supla_amazon_alexa_credentials_dao *dao =
        new supla_amazon_alexa_credentials_dao(dba, true);

    supla_alexa_discover_request *request = new supla_alexa_discover_request(
        user->getUserID(), supla_asynctask_queue::global_instance(),
        supla_voice_assistant_sync_thread_pool::global_instance(),
        user->amazonAlexaCredentials(),
        new supla_alexa_discover_payload_obtainer(dao));

    request->start();
  }
}
