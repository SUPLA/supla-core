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

#include "push/pn_delivery_task_thread_pool.h"

#include <string>

#include "asynctask/asynctask_queue.h"
#include "http/asynctask_http_thread_bucket.h"
#include "http/curl_adapter.h"
#include "svrcfg.h"

using std::string;

supla_pn_delivery_task_thread_pool
    *supla_pn_delivery_task_thread_pool::_global_instance = NULL;

supla_pn_delivery_task_thread_pool::supla_pn_delivery_task_thread_pool(
    supla_asynctask_queue *queue)
    : supla_asynctask_http_thread_pool(queue) {}

supla_pn_delivery_task_thread_pool::~supla_pn_delivery_task_thread_pool(void) {}

string supla_pn_delivery_task_thread_pool::pool_name(void) {
  return "PushNotificationPool";
}

unsigned int supla_pn_delivery_task_thread_pool::get_cfg_thread_count_limit(
    void) {
  return scfg_int(CFG_PUSH_THREAD_COUNT_LIMIT);
}

int supla_pn_delivery_task_thread_pool::get_cfg_requests_per_thread(void) {
  return scfg_int(CFG_PUSH_REQUESTS_PER_THREAD);
}

unsigned long long
supla_pn_delivery_task_thread_pool::get_cfg_keep_alive_time_usec(void) {
  return scfg_int(CFG_PUSH_THREAD_KEEP_ALIVE_SEC) * 1000000;
}

size_t supla_pn_delivery_task_thread_pool::get_cfg_keep_alive_max_thread_count(
    void) {
  return scfg_int(CFG_PUSH_KEEP_ALIVE_MAX_THREAD_COUNT);
}

// static
supla_pn_delivery_task_thread_pool *
supla_pn_delivery_task_thread_pool::global_instance(void) {
  if (_global_instance == NULL) {
    _global_instance = new supla_pn_delivery_task_thread_pool(
        supla_asynctask_queue::global_instance());
  }

  return _global_instance;
}
