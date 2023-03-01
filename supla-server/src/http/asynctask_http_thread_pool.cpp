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

#include "http/asynctask_http_thread_pool.h"

#include <string>

#include "asynctask/asynctask_queue.h"
#include "http/asynctask_http_thread_bucket.h"
#include "http/curl_adapter.h"
#include "svrcfg.h"

using std::string;

supla_asynctask_http_thread_pool
    *supla_asynctask_http_thread_pool::_global_instance = NULL;

supla_asynctask_http_thread_pool::supla_asynctask_http_thread_pool(
    supla_asynctask_queue *queue)
    : supla_abstract_asynctask_thread_pool(queue) {
  _thread_count_limit = scfg_int(CFG_HTTP_THREAD_COUNT_LIMIT);
  requests_per_thread = scfg_int(CFG_HTTP_REQUESTS_PER_THREAD);
}

supla_asynctask_http_thread_pool::~supla_asynctask_http_thread_pool(void) {}

unsigned int supla_asynctask_http_thread_pool::thread_count_limit(void) {
  return _thread_count_limit;
}

int supla_asynctask_http_thread_pool::tasks_per_thread(void) {
  return requests_per_thread;
}

string supla_asynctask_http_thread_pool::pool_name(void) { return "HttpPool"; }

supla_asynctask_thread_bucket *supla_asynctask_http_thread_pool::get_bucket(
    void) {
  return new supla_asynctask_http_thread_bucket(new supla_curl_adapter());
}

// static
supla_asynctask_http_thread_pool *
supla_asynctask_http_thread_pool::global_instance(void) {
  if (_global_instance == NULL) {
    _global_instance = new supla_asynctask_http_thread_pool(
        supla_asynctask_queue::global_instance());
  }

  return _global_instance;
}
