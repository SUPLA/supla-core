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

#include "http/httprequest.h"
#include <cstddef>  // NOLINT
#include <list>     // NOLINT
#include "http/httprequestqueue.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "sthread.h"

supla_http_request_queue *supla_http_request_queue::instance = NULL;

char supla_http_request_queue_arr_userspace_arr_queue_clean(void *_request) {
  supla_http_request *request = static_cast<supla_http_request *>(_request);
  if (request) {
    delete request;
    return 1;
  }

  return 0;
}

char supla_http_request_queue_arr_userspace_clean(void *_user_space) {
  _heq_user_space_t *user_space = static_cast<_heq_user_space_t *>(_user_space);
  if (user_space) {
    if (user_space->arr_queue) {
      safe_array_clean(user_space->arr_queue,
                       supla_http_request_queue_arr_userspace_arr_queue_clean);
      safe_array_free(user_space->arr_queue);
      delete user_space;
      return 1;
    }
  }

  return 0;
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
  arr_user_space = safe_array_init();
}

supla_http_request_queue::~supla_http_request_queue() {
  safe_array_clean(arr_user_space,
                   supla_http_request_queue_arr_userspace_clean);
  safe_array_free(arr_user_space);

  eh_free(main_eh);
}

void supla_http_request_queue::iterate(void *q_sthread) {
  while (sthread_isterminated(q_sthread) == 0) {
    eh_wait(main_eh, 30000000);
  }
}

_heq_user_space_t *supla_http_request_queue::getUserSpace(supla_user *user) {
  _heq_user_space_t *result = NULL;

  if (user == NULL) {
    return NULL;
  }

  safe_array_lock(arr_user_space);
  for (int a = 0; a < safe_array_count(arr_user_space); a++) {
    _heq_user_space_t *user_space =
        static_cast<_heq_user_space_t *>(safe_array_get(arr_user_space, a));
    if (user_space && user_space->user == user) {
      result = user_space;
      break;
    }
  }

  if (result == NULL) {
    result = new _heq_user_space_t();
    result->user = user;
    result->arr_queue = safe_array_init();
    if (result->arr_queue == NULL) {
      delete result;
    } else {
      safe_array_add(arr_user_space, result);
    }
  }

  safe_array_unlock(arr_user_space);

  return result;
}

void supla_http_request_queue::addRequest(_heq_user_space_t *user_space,
                                          supla_http_request *request) {
  safe_array_add(user_space->arr_queue, request);
}

void supla_http_request_queue::addRequest(supla_http_request *request) {
  supla_user *user = NULL;
  _heq_user_space_t *user_space = NULL;
  if (request && (user = request->getUser()) != NULL &&
      (user_space = getUserSpace(user)) != NULL) {
    addRequest(user_space, request);
  }
}

void supla_http_request_queue::onChannelChangeEvent(
    supla_user *user, int deviceId, int channelId, short eventSourceType,
    const char *correlationToken[]) {
  _heq_user_space_t *user_space = getUserSpace(user);
  if (user_space == NULL) {
    return;
  }

  std::list<supla_http_request *> requests =
      AbstractHttpRequestFactory::createByChannelEventSourceType(
          user, eventSourceType);

  for (std::list<supla_http_request *>::iterator it = requests.begin();
       it != requests.end(); it++) {
    supla_http_request *request = *it;

    request->setEventSourceType(eventSourceType);
    request->setDeviceId(deviceId);
    request->setChannelId(channelId);

    safe_array_lock(user_space->arr_queue);

    int ClassID = request->getClassID();

    for (int a = 0; a < safe_array_count(user_space->arr_queue); a++) {
      supla_http_request *existing = static_cast<supla_http_request *>(
          safe_array_get(user_space->arr_queue, a));
      if (existing && existing->getClassID() == ClassID &&
          existing->getDeviceId() == deviceId &&
          existing->getChannelId() == channelId &&
          !request->verifyExisting(existing)) {
        break;
      }
    }

    safe_array_unlock(user_space->arr_queue);

    if (request) {
      if (!request->isEventSourceTypeAccepted(eventSourceType, false) ||
          !request->queueUp()) {
        delete request;
        request = NULL;
      }

      if (request) {
        addRequest(user_space, request);
      }
    }
  }

  if (requests.size()) {
    eh_raise_event(main_eh);
  }
}

void http_request_queue_loop(void *ssd, void *q_sthread) {
  supla_http_request_queue::getInstance()->iterate(q_sthread);
}
