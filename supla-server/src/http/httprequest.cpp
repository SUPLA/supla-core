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

#include <stdlib.h>

#include "lck.h"
#include "log.h"
#include "sthread.h"
#include "string.h"
#include "svrcfg.h"

using std::function;
using std::list;

supla_http_request::supla_http_request(supla_user *user, int ClassID,
                                       int DeviceId, int ChannelId,
                                       event_type EventType,
                                       const supla_caller &Caller) {
  this->lck = lck_init();
  this->extraParams = NULL;
  this->http = NULL;
  this->https = NULL;
  this->user = user;
  this->EventType = EventType;
  this->Caller = Caller;
  this->ClassID = ClassID;
  this->DeviceId = DeviceId;
  this->ChannelId = ChannelId;
  this->timeoutUs = 0;
  this->startTime.tv_sec = 0;
  this->startTime.tv_usec = 0;
  this->touchTimeSec = 0;
  this->touchCount = 0;

  setTimeout(scfg_int(CFG_HTTP_REQUEST_TIMEOUT) * 1000);
  setDelay(0);
}

supla_http_request::~supla_http_request() {
  lck_lock(this->lck);
  if (extraParams) {
    delete extraParams;
    extraParams = NULL;
  }

  if (http) {
    delete http;
    http = NULL;
  }

  if (https) {
    delete https;
    https = NULL;
  }
  lck_unlock(this->lck);

  lck_free(this->lck);
}

supla_trivial_http *supla_http_request::getHttp() {
  supla_trivial_http *result = NULL;
  lck_lock(this->lck);
  if (!http) {
    http = new supla_trivial_http();
  }
  result = http;
  lck_unlock(this->lck);

  return result;
}

supla_trivial_https *supla_http_request::getHttps() {
  supla_trivial_https *result = NULL;
  lck_lock(this->lck);
  if (!https) {
    https = new supla_trivial_https();
  }
  result = https;
  lck_unlock(this->lck);

  return result;
}

int supla_http_request::getClassID(void) { return ClassID; }

supla_user *supla_http_request::getUser(void) { return user; }

int supla_http_request::getUserID(void) { return user ? user->getUserID() : 0; }

void supla_http_request::setEventType(event_type EventType) {
  this->EventType = EventType;
}

event_type supla_http_request::getEventType(void) { return EventType; }

void supla_http_request::setCaller(const supla_caller &Caller) {
  this->Caller = Caller;
}

const supla_caller &supla_http_request::getCaller(void) { return Caller; }

void supla_http_request::setDeviceId(int DeviceId) {
  this->DeviceId = DeviceId;
}

int supla_http_request::getDeviceId(void) { return DeviceId; }

bool supla_http_request::isDeviceIdEqual(int DeviceId) {
  return this->DeviceId == DeviceId;
}

void supla_http_request::setChannelId(int ChannelId) {
  this->ChannelId = ChannelId;
}

int supla_http_request::getChannelId(void) { return ChannelId; }

bool supla_http_request::isChannelIdEqual(int ChannelId) {
  return this->ChannelId == ChannelId;
}

void supla_http_request::setExtraParams(
    supla_http_request_extra_params *extraParams) {
  lck_lock(this->lck);
  if (this->extraParams) {
    delete this->extraParams;
  }
  if (extraParams) {
    this->extraParams = extraParams->clone();
  }
  lck_unlock(this->lck);
}

void supla_http_request::accessExtraParams(
    function<void(supla_http_request_extra_params *)> f) {
  lck_lock(this->lck);
  f(extraParams);
  lck_unlock(this->lck);
}

void supla_http_request::setDelay(unsigned long long delayUs) {
  gettimeofday(&startTime, NULL);

  if (delayUs > 0) {
    startTime.tv_sec += delayUs / 1000000;
    startTime.tv_usec += delayUs % 1000000;
  }

  supla_http_request_queue::getInstance()->recalculateTime();
}

void supla_http_request::setTimeout(unsigned long long timeoutUs) {
  this->timeoutUs = timeoutUs;
}

unsigned long long supla_http_request::getTimeout(void) {
  return this->timeoutUs;
}

unsigned long long supla_http_request::getStartTime(void) {
  return startTime.tv_sec;
}

long long supla_http_request::timeLeft(struct timeval *now) {
  struct timeval _now;
  if (!now) {
    gettimeofday(&_now, NULL);
    now = &_now;
  }

  if (now->tv_sec <= startTime.tv_sec) {
    return (startTime.tv_sec * (long long)1000000 + startTime.tv_usec) -
           (now->tv_sec * (long long)1000000 + now->tv_usec);
  }

  return 0;
}

bool supla_http_request::isWaiting(struct timeval *now) {
  return timeLeft(now) > 0;
}

bool supla_http_request::timeout(struct timeval *now) {
  if (timeoutUs == 0) {
    return false;
  }

  struct timeval _now;
  if (!now) {
    gettimeofday(&_now, NULL);
    now = &_now;
  }

  struct timeval tout = startTime;

  tout.tv_sec += timeoutUs / 1000000;
  tout.tv_usec += timeoutUs % 1000000 * 1000;

  if (now->tv_sec < tout.tv_sec) {
    return false;
  }

  if (now->tv_sec == tout.tv_sec && now->tv_usec < tout.tv_usec) {
    return false;
  }

  return true;
}

void supla_http_request::touch(struct timeval *now) {
  touchCount++;
  if (now) {
    touchTimeSec = now->tv_sec;
  }
}

unsigned long long supla_http_request::getTouchTimeSec(void) {
  return touchTimeSec;
}

unsigned long long supla_http_request::getTouchCount(void) {
  return touchCount;
}

void supla_http_request::terminate(void *sthread) {
  if (sthread) {
    sthread_terminate(sthread);
  }

  lck_lock(this->lck);
  if (https) {
    https->terminate();
  }

  if (http) {
    http->terminate();
  }
  lck_unlock(this->lck);
}

void supla_http_request::requestWillBeAdded(void) {}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

AbstractHttpRequestFactory::AbstractHttpRequestFactory(void) {
  getFactories().push_back(this);
  ClassID = getFactories().size();
}

AbstractHttpRequestFactory::~AbstractHttpRequestFactory(void) {
  getFactories().remove(this);
}

list<AbstractHttpRequestFactory *> &AbstractHttpRequestFactory::getFactories(
    void) {
  static list<AbstractHttpRequestFactory *> factories;
  return factories;
}

int AbstractHttpRequestFactory::getClassID(void) { return ClassID; }

// static
list<supla_http_request *> AbstractHttpRequestFactory::createInTheCallerContext(
    supla_user *user, int DeviceId, int ChannelId, event_type EventType,
    const supla_caller &Caller) {
  list<AbstractHttpRequestFactory *> factories = getFactories();
  list<supla_http_request *> result;
  for (auto it = factories.begin(); it != factories.end(); it++) {
    supla_http_request *request = (*it)->create(
        user, (*it)->getClassID(), DeviceId, ChannelId, EventType, Caller);
    if (request) {
      if (request->isEventTypeAccepted(EventType, true) &&
          request->isCallerAccepted(Caller, true)) {
        result.push_back(request);
      } else {
        delete request;
        request = NULL;
      }
    }
  }
  return result;
}
