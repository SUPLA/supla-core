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

#include "http/httprequestqueue.h"

#include <unistd.h>  // NOLINT

#include <cstddef>  // NOLINT
#include <list>     // NOLINT

#include "database.h"
#include "http/httprequest.h"
#include "http/httprequestvoiceassistantextraparams.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "serverstatus.h"
#include "sthread.h"
#include "svrcfg.h"
#include "tools.h"
#include "user/user.h"

// TODO(anyone): Pass the http queue handling to supla_asynctasks

typedef struct {
  supla_http_request *request;
  void *sthread;
  void *arr_thread;
} _request_thread_ptr_t;

supla_http_request_queue *supla_http_request_queue::instance = NULL;

char supla_http_request_queue_arr_queue_clean(void *_request) {
  supla_http_request *request = static_cast<supla_http_request *>(_request);
  if (request) {
    delete request;
    return 1;
  }

  return 0;
}

void supla_http_request_thread_execute(void *ptr, void *sthread) {
  database::thread_init();
  static_cast<_request_thread_ptr_t *>(ptr)->request->execute(sthread);
}

void supla_http_request_thread_finish(void *_ptr, void *sthread) {
  _request_thread_ptr_t *ptr = static_cast<_request_thread_ptr_t *>(_ptr);
  void *arr_thread = ptr->arr_thread;

  safe_array_lock(arr_thread);
  delete ptr->request;
  delete ptr;
  safe_array_remove(arr_thread, ptr);
  safe_array_unlock(arr_thread);

  supla_http_request_queue::getInstance()->raiseEvent();
  database::thread_end();
}

// static
void supla_http_request_queue::init(void) {
  supla_http_request_queue::instance = new supla_http_request_queue();
}

// static
void supla_http_request_queue::queueFree(void) {
  if (supla_http_request_queue::instance) {
    delete supla_http_request_queue::instance;
    supla_http_request_queue::instance = NULL;
  }
}

// static
supla_http_request_queue *supla_http_request_queue::getInstance(void) {
  return supla_http_request_queue::instance;
}

supla_http_request_queue::supla_http_request_queue() {
  this->main_eh = eh_init();
  this->lck = lck_init();
  this->arr_queue = safe_array_init();
  this->arr_thread = safe_array_init();
  this->thread_count_limit = scfg_int(CFG_HTTP_THREAD_COUNT_LIMIT);
  this->queue_offset = 0;
  this->last_user_id = 0;
  this->last_iterate_time_sec = 0;
  this->time_of_the_next_iteration_usec = 0;
  this->request_total_count = 0;
  this->last_metric_log_time_sec = 0;
}

void supla_http_request_queue::terminateAllThreads(void) {
  safe_array_lock(arr_thread);
  for (int a = 0; a < safe_array_count(arr_thread); a++) {
    _request_thread_ptr_t *ptr =
        static_cast<_request_thread_ptr_t *>(safe_array_get(arr_thread, a));
    if (ptr && ptr->request) {
      ptr->request->terminate(ptr->sthread);
    }
  }
  safe_array_unlock(arr_thread);
}

supla_http_request_queue::~supla_http_request_queue() {
  safe_array_free(arr_thread);

  safe_array_clean(arr_queue, supla_http_request_queue_arr_queue_clean);
  safe_array_free(arr_queue);

  lck_free(lck);
  eh_free(main_eh);
}

void supla_http_request_queue::runThread(supla_http_request *request) {
  if (!request) {
    return;
  }

  _request_thread_ptr_t *ptr = new _request_thread_ptr_t();
  if (ptr) {
    safe_array_lock(arr_thread);
    safe_array_add(arr_thread, ptr);

    Tsthread_params stp;

    stp.execute = supla_http_request_thread_execute;
    stp.finish = supla_http_request_thread_finish;
    stp.user_data = ptr;
    stp.free_on_finish = true;
    stp.initialize = NULL;

    ptr->arr_thread = arr_thread;
    ptr->request = request;

    ptr->sthread = sthread_run(&stp);
    safe_array_unlock(arr_thread);

    lck_lock(lck);
    request_total_count++;
    lck_unlock(lck);
  }
}

supla_http_request *supla_http_request_queue::queuePop(void *q_sthread,
                                                       struct timeval *now) {
  supla_http_request *result = NULL;

  safe_array_lock(arr_queue);

  if (queue_offset >= safe_array_count(arr_queue)) {
    queue_offset = 0;
    last_user_id = 0;
  }

  while (queue_offset < safe_array_count(arr_queue)) {
    supla_http_request *request = static_cast<supla_http_request *>(
        safe_array_get(arr_queue, queue_offset));

    if (request && request->getUserID() != last_user_id &&
        !request->isWaiting(now)) {
      if (request->isCancelled(q_sthread)) {
        delete request;
        request = NULL;
      } else if (request->timeout(NULL)) {
        supla_log(LOG_WARNING,
                  "HTTP request execution timeout! UserID: %i, IODevice: %i "
                  "Channel: %i QS: %i, TC: %i,"
                  "EventSourceType: %i (%lu/%lu/%lu/%lu/%lu/%lu/%i)",
                  request->getUserID(), request->getDeviceId(),
                  request->getChannelId(), queueSize(), threadCount(),
                  request->getEventSourceType(), request->getTimeout(),
                  request->getStartTime(), now->tv_sec,
                  request->getTouchTimeSec(), request->getTouchCount(),
                  last_iterate_time_sec, queue_offset);

        delete request;
        request = NULL;
      } else {
        result = request;
      }

      safe_array_delete(arr_queue, queue_offset);
      break;
    }

    if (request) {
      request->touch(now);
    }

    queue_offset++;

    if (result) {
      last_user_id = result->getUserID();
      break;
    }
  }
  safe_array_unlock(arr_queue);

  return result;
}

int supla_http_request_queue::queueSize(void) {
  return safe_array_count(arr_queue);
}

int supla_http_request_queue::threadCount(void) {
  return safe_array_count(arr_thread);
}

int supla_http_request_queue::threadCountLimit(void) {
  return thread_count_limit;
}

unsigned long long supla_http_request_queue::requestTotalCount(void) {
  unsigned long long result = 0;
  lck_lock(lck);
  result = request_total_count;
  lck_unlock(lck);
  return result;
}

void supla_http_request_queue::raiseEvent(void) { eh_raise_event(main_eh); }

void supla_http_request_queue::iterate(void *q_sthread) {
  bool warn_msg = false;
  while (sthread_isterminated(q_sthread) == 0) {
    struct timeval now;
    gettimeofday(&now, NULL);

    if (last_iterate_time_sec > 0 && now.tv_sec - last_iterate_time_sec >= 5) {
      supla_log(LOG_WARNING, "Http request - long iteration time (%i)",
                now.tv_sec - last_iterate_time_sec);
    }

    lck_lock(lck);
    last_iterate_time_sec = now.tv_sec;
    lck_unlock(lck);

    if (queueSize() > 0) {
      supla_http_request *request = NULL;

      if (threadCount() < threadCountLimit()) {
        warn_msg = false;
        request = queuePop(q_sthread, &now);
        if (request) {
          runThread(request);
          recalculateTime(&now);
        }
      } else if (!warn_msg) {
        supla_log(LOG_WARNING,
                  "Http request - thread count limit exceeded (%i)",
                  threadCountLimit());
        warn_msg = true;
      } else {
        eh_wait(main_eh, 10000);
      }
    } else {
      recalculateTime(&now);
    }

    long long wait_time = 0;

    lck_lock(lck);
    wait_time =
        time_of_the_next_iteration_usec - (now.tv_sec * 1000000 + now.tv_usec);
    lck_unlock(lck);

    if (wait_time > 0) {
      if (wait_time > 2000000) {
        wait_time = 2000000;
      }
      eh_wait(main_eh, wait_time);
    }
  }

  terminateAllThreads();

  int n_wait = 1;
  while (threadCount()) {
    if (n_wait == 20) {
      supla_log(LOG_INFO, "Waiting for http threads...");
    }
    usleep(100000);
    n_wait++;
  }
}

void supla_http_request_queue::logStuckWarning(void) {
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
  struct timeval now;
  gettimeofday(&now, NULL);

  lck_lock(lck);
  int time = now.tv_sec - last_iterate_time_sec;
  lck_unlock(lck);

  if (time > 10) {
    supla_log(LOG_WARNING, "HTTP Queue iteration is stuck!");
  }
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
}

void supla_http_request_queue::logMetrics(unsigned int min_interval_sec) {
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);

  if (min_interval_sec > 0) {
    struct timeval now;
    gettimeofday(&now, NULL);
    if (last_metric_log_time_sec == 0) {
      last_metric_log_time_sec = now.tv_sec;
    }
    if (now.tv_sec - last_metric_log_time_sec < min_interval_sec) {
      return;
    }
    last_metric_log_time_sec = now.tv_sec;
  }

  supla_log(LOG_INFO,
            "HTTP QUEUE METRICS: CURRENT[Thread Count: %i, Queue Size: %i] "
            "TOTAL[Request Count: %lu]",
            threadCount(), queueSize(), requestTotalCount());
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
}

void supla_http_request_queue::recalculateTime(struct timeval *now) {
  unsigned long long now_usec = now->tv_sec * 1000000 + now->tv_usec;
  long long time = 2000000;

  safe_array_lock(arr_queue);

  for (int b = 0; b < safe_array_count(arr_queue) && time > 0; b++) {
    supla_http_request *request =
        static_cast<supla_http_request *>(safe_array_get(arr_queue, b));

    if (request) {
      long long timeLeft = request->timeLeft(now);
      if (timeLeft < time) {
        time = timeLeft;
      }
    }
  }
  safe_array_unlock(arr_queue);

  lck_lock(lck);
  time_of_the_next_iteration_usec = now_usec + (time > 0 ? time : 0);
  lck_unlock(lck);

  if (queueSize() > 0) {
    raiseEvent();
  }
}

void supla_http_request_queue::recalculateTime(void) {
  struct timeval now;
  gettimeofday(&now, NULL);
  recalculateTime(&now);
}

void supla_http_request_queue::addRequest(supla_http_request *request) {
  safe_array_add(arr_queue, request);
  recalculateTime();
}

void supla_http_request_queue::createByChannelEventSourceType(
    supla_user *user, int deviceId, int channelId, event_type eventType,
    event_source_type eventSourceType,
    supla_http_request_extra_params *extraParams) {
  if (st_app_terminate != 0) {
    if (extraParams) {
      delete extraParams;
    }
    return;
  }

  std::list<supla_http_request *> requests =
      AbstractHttpRequestFactory::createByChannelEventSourceType(
          user, deviceId, channelId, eventType, eventSourceType);

  for (std::list<supla_http_request *>::iterator it = requests.begin();
       it != requests.end(); it++) {
    supla_http_request *request = *it;

    safe_array_lock(arr_queue);

    int ClassID = request->getClassID();

    for (int a = 0; a < safe_array_count(arr_queue); a++) {
      supla_http_request *existing =
          static_cast<supla_http_request *>(safe_array_get(arr_queue, a));
      if (existing && existing->getClassID() == ClassID &&
          existing->isDeviceIdEqual(deviceId) &&
          existing->isChannelIdEqual(channelId) &&
          !request->verifyExisting(existing)) {
        break;
      }
    }

    safe_array_unlock(arr_queue);

    if (request) {
      if (!request->isEventSourceTypeAccepted(eventSourceType, false) ||
          !request->queueUp()) {
        delete request;
        request = NULL;
      }

      if (request) {
        request->setExtraParams(extraParams);
        extraParams = NULL;
        request->requestWillBeAdded();
        addRequest(request);
      }
    }
  }

  if (extraParams) {
    delete extraParams;
  }
}

void supla_http_request_queue::onChannelValueChangeEvent(
    supla_user *user, int deviceId, int channelId,
    event_source_type eventSourceType, const char correlationToken[],
    const char googleRequestId[]) {
  createByChannelEventSourceType(
      user, deviceId, channelId, ET_CHANNEL_VALUE_CHANGED, eventSourceType,
      new supla_http_request_voice_assistant_extra_params(correlationToken,
                                                          googleRequestId));
}

void supla_http_request_queue::onChannelsAddedEvent(
    supla_user *user, int deviceId, event_source_type eventSourceType,
    const char correlationToken[], const char googleRequestId[]) {
  createByChannelEventSourceType(
      user, deviceId, 0, ET_CHANNELS_ADDED, eventSourceType,
      new supla_http_request_voice_assistant_extra_params(correlationToken,
                                                          googleRequestId));
}

void supla_http_request_queue::onDeviceDeletedEvent(
    supla_user *user, int deviceId, event_source_type eventSourceType,
    const char correlationToken[], const char googleRequestId[]) {
  createByChannelEventSourceType(
      user, deviceId, 0, ET_DEVICE_DELETED, eventSourceType,
      new supla_http_request_voice_assistant_extra_params(correlationToken,
                                                          googleRequestId));
}

void supla_http_request_queue::onUserReconnectEvent(
    supla_user *user, event_source_type eventSourceType) {
  createByChannelEventSourceType(user, 0, 0, ET_USER_RECONNECT, eventSourceType,
                                 NULL);
}

void supla_http_request_queue::onGoogleHomeSyncNeededEvent(
    supla_user *user, event_source_type eventSourceType) {
  createByChannelEventSourceType(user, 0, 0, ET_GOOGLE_HOME_SYNC_NEEDED,
                                 eventSourceType, NULL);
}

void supla_http_request_queue::onActionsTriggered(supla_user *user,
                                                  int deviceId, int channelId,
                                                  unsigned int actions) {}

void http_request_queue_loop(void *ssd, void *q_sthread) {
  supla_http_request_queue::getInstance()->iterate(q_sthread);
}
