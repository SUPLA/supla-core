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

supla_http_request::supla_http_request(supla_user *user, int ClassID,
                                       int DeviceId, int ChannelId,
                                       event_type EventType,
                                       event_source_type EventSourceType) {
  this->lck = lck_init();
  this->http = NULL;
  this->https = NULL;
  this->user = user;
  this->EventType = EventType;
  this->EventSourceType = EventSourceType;
  this->ClassID = ClassID;
  this->DeviceId = DeviceId;
  this->ChannelId = ChannelId;
  this->timeoutUs = 0;
  this->startTime.tv_sec = 0;
  this->startTime.tv_usec = 0;
  this->correlationToken = NULL;
  this->googleRequestId = NULL;

  setTimeout(scfg_int(CFG_HTTP_REQUEST_TIMEOUT) * 1000);
  setDelay(0);
}

supla_http_request::~supla_http_request() {
  if (correlationToken) {
    free(correlationToken);
    correlationToken = NULL;
  }

  if (googleRequestId) {
    free(googleRequestId);
    googleRequestId = NULL;
  }

  lck_lock(this->lck);
  if (http) {
    delete http;
  }

  if (https) {
    delete https;
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

void supla_http_request::setEventType(event_type EventType) {
  this->EventType = EventType;
}

event_type supla_http_request::getEventType(void) { return EventType; }

void supla_http_request::setEventSourceType(event_source_type EventSourceType) {
  this->EventSourceType = EventSourceType;
}

event_source_type supla_http_request::getEventSourceType(void) {
  return EventSourceType;
}

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

void supla_http_request::setCorrelationToken(const char correlationToken[]) {
  lck_lock(this->lck);
  if (this->correlationToken) {
    free(this->correlationToken);
    this->correlationToken = NULL;
  }

  if (correlationToken &&
      strnlen(correlationToken, CORRELATIONTOKEN_MAXSIZE) > 0) {
    this->correlationToken =
        strndup(correlationToken, CORRELATIONTOKEN_MAXSIZE);
  }
  lck_unlock(this->lck);
}

const char *supla_http_request::getCorrelationTokenPtr(void) {
  char *result = NULL;
  lck_lock(this->lck);
  result = correlationToken;
  lck_unlock(this->lck);
  return result;
}

void supla_http_request::setGoogleRequestId(const char googleRequestId[]) {
  lck_lock(this->lck);
  if (this->googleRequestId) {
    free(this->googleRequestId);
    this->googleRequestId = NULL;
  }

  if (googleRequestId &&
      strnlen(googleRequestId, GOOGLEREQUESTID_MAXSIZE) > 0) {
    this->googleRequestId = strndup(googleRequestId, GOOGLEREQUESTID_MAXSIZE);
  }
  lck_unlock(this->lck);
}

const char *supla_http_request::getGoogleRequestIdPtr(void) {
  char *result = NULL;
  lck_lock(this->lck);
  result = googleRequestId;
  lck_unlock(this->lck);
  return result;
}

void supla_http_request::setDelay(int delayUs) {
  gettimeofday(&startTime, NULL);

  if (delayUs > 0) {
    startTime.tv_sec += delayUs / 1000000;
    startTime.tv_usec += delayUs % 1000000;
  }
}

void supla_http_request::setTimeout(int timeoutUs) {
  this->timeoutUs = timeoutUs;
}

int supla_http_request::getTimeout(void) { return this->timeoutUs; }

int supla_http_request::getStartTime(void) { return startTime.tv_sec; }

int supla_http_request::timeLeft(struct timeval *now) {
  struct timeval _now;
  if (!now) {
    gettimeofday(&_now, NULL);
    now = &_now;
  }

  if (now->tv_sec <= startTime.tv_sec) {
    int us = (startTime.tv_sec - now->tv_sec) * 1000000;
    us += startTime.tv_usec - now->tv_usec;
    return us;
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

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

std::list<AbstractHttpRequestFactory *> AbstractHttpRequestFactory::factories;

AbstractHttpRequestFactory::AbstractHttpRequestFactory(void) {
  factories.push_back(this);
  ClassID = factories.size();
}

AbstractHttpRequestFactory::~AbstractHttpRequestFactory(void) {
  factories.remove(this);
}

int AbstractHttpRequestFactory::getClassID(void) { return ClassID; }

// static
std::list<supla_http_request *>
AbstractHttpRequestFactory::createByChannelEventSourceType(
    supla_user *user, int DeviceId, int ChannelId, event_type EventType,
    event_source_type EventSourceType) {
  std::list<supla_http_request *> result;
  for (std::list<AbstractHttpRequestFactory *>::iterator it =
           AbstractHttpRequestFactory::factories.begin();
       it != AbstractHttpRequestFactory::factories.end(); it++) {
    supla_http_request *request =
        (*it)->create(user, (*it)->getClassID(), DeviceId, ChannelId, EventType,
                      EventSourceType);
    if (request) {
      if (request->isEventTypeAccepted(EventType, true) &&
          request->isEventSourceTypeAccepted(EventSourceType, true)) {
        result.push_back(request);
      } else {
        delete request;
        request = NULL;
      }
    }
  }
  return result;
}
