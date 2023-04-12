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
    supla_push_notification_gateway_credentials *fcm_credentials,
    supla_push_notification_gateway_credentials *apns_credentials)
    : supla_asynctask_http_request(supla_caller(), user_id, device_id,
                                   channel_id, queue, pool, nullptr) {
  this->push = push;
  this->fcm_credentials = fcm_credentials;
  this->apns_credentials = apns_credentials;
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

  {
    supla_fcm_client client(curl_adapter, fcm_credentials, push);
    if (client.send()) {
      result = true;
    }
  }

  {
    supla_apns_client client(curl_adapter, apns_credentials, push);
    if (client.send()) {
      result = true;
    }
  }

  return result;
}
