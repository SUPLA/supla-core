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

#include "push_notification_delivery_task.h"

#include "push/apns_client.h"
#include "push/fcm_client.h"

supla_push_notification_delivery_task::supla_push_notification_delivery_task(
    const supla_caller &caller, int user_id, int device_id, int channel_id,
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_push_notification *push,
    supla_access_token_providers *token_providers)
    : supla_asynctask_http_request(supla_caller(), user_id, device_id,
                                   channel_id, queue, pool, nullptr) {
  this->push = push;
  this->token_providers = token_providers;
}

supla_push_notification_delivery_task::~supla_push_notification_delivery_task(
    void) {
  if (push) {
    delete push;
  }
}

bool supla_push_notification_delivery_task::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  bool result = false;

  if (push->get_recipients().any_recipient_exists(platform_android)) {
    supla_fcm_client client(curl_adapter, token_providers, push);
    if (client.send()) {
      result = true;
    }
  }

  if (push->get_recipients().any_recipient_exists(platform_ios)) {
    supla_apns_client client(curl_adapter, token_providers, push);
    if (client.send()) {
      result = true;
    }
  }

  return result;
}
