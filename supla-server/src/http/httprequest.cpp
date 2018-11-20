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
#include "log.h"
#include "string.h"

supla_http_request::supla_http_request(supla_user *user, int ClassID) {
  this->user = user;
  this->EventSourceType = 0;
  this->ClassID = ClassID;
  this->DeviceId = 0;
  this->ChannelId = 0;
  this->timeoutMs = 0;
  this->startTime.tv_sec = 0;
  this->startTime.tv_usec = 0;
  this->correlationToken = NULL;
}

supla_http_request::~supla_http_request() {
  if (correlationToken) {
    free(correlationToken);
    correlationToken = NULL;
  }
}

int supla_http_request::getClassID(void) { return ClassID; }

supla_user *supla_http_request::getUser(void) { return user; }

void supla_http_request::setEventSourceType(short EventSourceType) {
  this->EventSourceType = EventSourceType;
}

short supla_http_request::getEventSourceType(void) { return EventSourceType; }

void supla_http_request::setDeviceId(int DeviceId) {
  this->DeviceId = DeviceId;
}

int supla_http_request::getDeviceId(void) { return DeviceId; }

void supla_http_request::setChannelId(int ChannelId) {
  this->ChannelId = ChannelId;
}

int supla_http_request::getChannelId(void) { return ChannelId; }

void supla_http_request::setCorrelationToken(const char correlationToken[]) {
  if (this->correlationToken) {
    free(this->correlationToken);
    this->correlationToken = NULL;
  }

  if (correlationToken &&
      strnlen(correlationToken, CORRELATIONTOKEN_MAXSIZE) > 0) {
    this->correlationToken =
        strndup(correlationToken, CORRELATIONTOKEN_MAXSIZE);
  }
}

const char *supla_http_request::getCorrelationTokenPtr(void) {
  return correlationToken;
}

void supla_http_request::setDelay(int delayMs) {
  if (delayMs > 0) {
    gettimeofday(&startTime, NULL);
    startTime.tv_sec += delayMs / 1000;
    startTime.tv_usec += (delayMs % 1000) * 1000;
  } else {
    startTime.tv_sec = 0;
    startTime.tv_usec = 0;
  }
}

void supla_http_request::setTimeout(int timeoutMs) {
  this->timeoutMs = timeoutMs;
}

int supla_http_request::timeLeft(struct timeval *now) {
  if (startTime.tv_sec == 0 && startTime.tv_usec == 0) {
    return 0;
  }

  struct timeval _now;
  if (!now) {
    gettimeofday(&_now, NULL);
    now = &_now;
  }

  if (now->tv_sec <= startTime.tv_sec) {
    int ms = (startTime.tv_sec - now->tv_sec) * 1000;
    ms += startTime.tv_usec / 1000 - now->tv_usec / 1000;
    return ms;
  }

  return 0;
}

bool supla_http_request::isWaiting(struct timeval *now) {
  return timeLeft(now) > 0;
}

bool supla_http_request::timeout(struct timeval *now) {
  if (timeoutMs == 0 || (startTime.tv_sec == 0 && startTime.tv_usec == 0)) {
    return false;
  }

  struct timeval _now;
  if (!now) {
    gettimeofday(&_now, NULL);
    now = &_now;
  }

  struct timeval tout = startTime;

  tout.tv_sec += timeoutMs / 1000;
  tout.tv_usec += (timeoutMs % 1000) * 1000;

  if (now->tv_sec > tout.tv_sec) {
    return false;
  }

  if (now->tv_sec == tout.tv_sec && now->tv_usec >= tout.tv_usec) {
    return false;
  }

  return true;
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
    supla_user *user, short EventSourceType) {
  std::list<supla_http_request *> result;
  for (std::list<AbstractHttpRequestFactory *>::iterator it =
           AbstractHttpRequestFactory::factories.begin();
       it != AbstractHttpRequestFactory::factories.end(); it++) {
    supla_http_request *request = (*it)->create(user, (*it)->getClassID());
    if (request) {
      if (request->isEventSourceTypeAccepted(EventSourceType, true)) {
        result.push_back(request);
      } else {
        delete request;
        request = NULL;
      }
    }
  }
  return result;
}
