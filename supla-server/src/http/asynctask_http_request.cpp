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
#include "log.h"
#include "metrics.h"
#include "svrcfg.h"

using std::function;
using std::shared_ptr;

supla_asynctask_http_request::supla_asynctask_http_request(
    const supla_caller &caller, int user_id, int device_id, int channel_id,
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_channel_property_getter *property_getter)
    : supla_abstract_asynctask(queue, pool) {
  this->caller = caller;
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;
  this->property_getter = property_getter;

  set_timeout(scfg_int(CFG_HTTP_REQUEST_TIMEOUT) * 1000);
}

supla_asynctask_http_request::~supla_asynctask_http_request(void) {
  if (property_getter) {
    delete property_getter;
  }
}

long long supla_asynctask_http_request::get_cfg_delay_warning_time_usec(void) {
  return scfg_int(CFG_HTTP_DELAY_WARNING_TIME) * 1000;
}

unsigned long long supla_asynctask_http_request::get_cfg_long_request_time_usec(
    void) {
  return scfg_int(CFG_HTTP_LONG_REQUEST_TIME) * 1000;
}

void supla_asynctask_http_request::on_timeout(
    unsigned long long timeout_usec, unsigned long long usec_after_timeout,
    bool log_allowed) {
  supla_abstract_asynctask::on_timeout(timeout_usec, usec_after_timeout, false);

  if (log_allowed) {
    supla_log(LOG_WARNING,
              "%s - HTTP Request timeout. ChannelId: %i, TimeoutUSec: "
              "%llu+%llu, TimeSinceExecReq: %lld",
              get_name().c_str(), get_channel_id(), timeout_usec,
              usec_after_timeout, time_since_exec_request_usec(nullptr));
  }
}

const supla_caller &supla_asynctask_http_request::get_caller(void) {
  return caller;
}

int supla_asynctask_http_request::get_user_id(void) { return user_id; }

int supla_asynctask_http_request::get_device_id(void) { return device_id; }

int supla_asynctask_http_request::get_channel_id(void) { return channel_id; }

supla_abstract_channel_property_getter *
supla_asynctask_http_request::get_property_getter(void) {
  return property_getter;
}

supla_abstract_channel_value *supla_asynctask_http_request::get_channel_value(
    int *func, supla_channel_availability_status *status) {
  if (func) {
    *func = 0;
  }

  if (status) {
    status->set_offline(true);
  }

  if (property_getter) {
    return property_getter->get_value(user_id, device_id, channel_id, func,
                                      status);
  }

  return nullptr;
}

supla_abstract_channel_value *supla_asynctask_http_request::get_channel_value(
    supla_channel_fragment *fragment,
    supla_channel_availability_status *status) {
  if (status) {
    status->set_offline(true);
  }

  if (property_getter) {
    return property_getter->get_value(user_id, device_id, channel_id, fragment,
                                      status);
  }

  return nullptr;
}

supla_channel_extended_value *
supla_asynctask_http_request::get_channel_extended_value(void) {
  if (property_getter) {
    return property_getter->get_extended_value(user_id, device_id, channel_id);
  }

  return nullptr;
}

bool supla_asynctask_http_request::_execute(
    bool *execute_again, supla_asynctask_thread_bucket *bucket) {
  supla_asynctask_http_thread_bucket *ht_bucket = nullptr;
  if (bucket) {
    ht_bucket = dynamic_cast<supla_asynctask_http_thread_bucket *>(bucket);
    if (ht_bucket && ht_bucket->get_adapter()) {
      if (get_cfg_delay_warning_time_usec()) {
        long long time_left = time_left_usec(nullptr);
        if (time_left < 0) {
          time_left *= -1;
          if (time_left >= get_cfg_delay_warning_time_usec()) {
            supla_log(LOG_WARNING,
                      "%s - The http request is delayed. ChannelId: %i, "
                      "DelayUSec: %lld, TimeSinceExecReq: %lld",
                      get_name().c_str(), get_channel_id(), time_left,
                      time_since_exec_request_usec(nullptr));
          }
        }
      }
      bool result = false;

      if (get_cfg_long_request_time_usec()) {
        unsigned long long time_usec = supla_metrics::measure_the_time_in_usec(
            [ht_bucket, &result, this]() -> void {
              result = make_request(ht_bucket->get_adapter());
            });

        if (time_usec >= get_cfg_long_request_time_usec()) {
          supla_log(LOG_WARNING,
                    "%s - Long time to complete the request. ChannelId: %i, "
                    "TimeUSec: %lld",
                    get_name().c_str(), get_channel_id(), time_usec);
        }

      } else {
        result = make_request(ht_bucket->get_adapter());
      }

      return result;
    }
  }

  return false;
}
