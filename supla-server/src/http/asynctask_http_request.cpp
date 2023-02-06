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

#include "http/asynctask_http_request.h"

#include "http/asynctask_http_thread_bucket.h"
#include "svrcfg.h"

using std::function;
using std::shared_ptr;

supla_asynctask_http_request::supla_asynctask_http_request(
    const supla_caller &caller, supla_user *user, int device_id, int channel_id,
    event_type et, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool)
    : supla_abstract_asynctask(queue, pool) {
  this->caller = caller;
  this->user = user;
  this->device_id = device_id;
  this->channel_id = channel_id;
  this->et = et;
  set_timeout(scfg_int(CFG_HTTP_REQUEST_TIMEOUT) * 1000);
}

supla_asynctask_http_request::~supla_asynctask_http_request(void) {}

const supla_caller &supla_asynctask_http_request::get_caller(void) {
  return caller;
}

supla_user *supla_asynctask_http_request::get_user(void) { return user; }

int supla_asynctask_http_request::get_device_id(void) { return device_id; }

int supla_asynctask_http_request::get_channel_id(void) { return channel_id; }

void supla_asynctask_http_request::access_device(
    function<void(shared_ptr<supla_device>)> on_device) {
  shared_ptr<supla_device> device =
      get_user()->get_devices()->get(get_device_id(), get_channel_id());
  if (device != nullptr) {
    on_device(device);
    device = nullptr;
  }
}

event_type supla_asynctask_http_request::get_event_type(void) { return et; }

bool supla_asynctask_http_request::_execute(
    bool *execute_again, supla_asynctask_thread_bucket *bucket) {
  supla_asynctask_http_thread_bucket *ht_bucket = nullptr;
  if (bucket) {
    ht_bucket = dynamic_cast<supla_asynctask_http_thread_bucket *>(bucket);
    if (ht_bucket && ht_bucket->get_adapter()) {
      return make_request(ht_bucket->get_adapter());
    }
  }

  return false;
}
