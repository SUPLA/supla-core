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

#ifndef HTTP_HTTPREQUEST_H_
#define HTTP_HTTPREQUEST_H_

#include <sys/time.h>

#include <cstddef>
#include <functional>
#include <list>
#include <string>

#include "http/httprequestextraparams.h"
#include "http/httprequestqueue.h"
#include "http/trivialhttps.h"
#include "user/user.h"

class supla_http_request {
 private:
  void *lck;
  supla_http_request_extra_params *extraParams;
  supla_trivial_http *http;
  supla_trivial_https *https;
  supla_user *user;
  int ClassID;
  event_type EventType;
  supla_caller Caller;
  int DeviceId;
  int ChannelId;
  struct timeval startTime;
  unsigned long long timeoutUs;
  unsigned long long touchTimeSec;
  unsigned long long touchCount;

 protected:
  supla_trivial_http *getHttp();
  supla_trivial_https *getHttps();

 public:
  supla_http_request(supla_user *user, int ClassID, int DeviceId, int ChannelId,
                     event_type EventType, const supla_caller &Caller);
  int getClassID(void);
  supla_user *getUser(void);
  int getUserID(void);
  void setCaller(const supla_caller &Caller);
  const supla_caller &getCaller(void);
  void setEventType(event_type EventType);
  event_type getEventType(void);
  void setDeviceId(int DeviceId);
  int getDeviceId(void);
  virtual bool isDeviceIdEqual(int DeviceId);
  virtual void setChannelId(int ChannelId);
  int getChannelId(void);
  virtual bool isChannelIdEqual(int ChannelId);
  void setExtraParams(supla_http_request_extra_params *extraParams);
  void accessExtraParams(
      std::function<void(supla_http_request_extra_params *)> f);
  void setDelay(unsigned long long delayUs);
  void setTimeout(unsigned long long timeoutUs);
  unsigned long long getTimeout(void);
  unsigned long long getStartTime(void);
  long long timeLeft(struct timeval *now);
  bool isWaiting(struct timeval *now);
  bool timeout(struct timeval *now);
  void touch(struct timeval *now);
  unsigned long long getTouchTimeSec(void);
  unsigned long long getTouchCount(void);

  virtual bool isCancelled(void *sthread) = 0;
  virtual bool verifyExisting(supla_http_request *existing) = 0;
  virtual bool queueUp(void) = 0;
  virtual bool isCallerAccepted(const supla_caller &caller,
                                bool verification) = 0;
  virtual bool isEventTypeAccepted(event_type eventType, bool verification) = 0;
  virtual void execute(void *sthread) = 0;
  virtual void terminate(void *sthread);
  virtual void requestWillBeAdded(void);
  virtual ~supla_http_request();
};

class AbstractHttpRequestFactory {
 private:
  int ClassID;
  virtual supla_http_request *create(supla_user *user, int ClassID,
                                     int DeviceId, int ChannelId,
                                     event_type EventType,
                                     const supla_caller &caller) = 0;

  static std::list<AbstractHttpRequestFactory *> &getFactories(void);

 public:
  AbstractHttpRequestFactory(void);
  virtual ~AbstractHttpRequestFactory(void);
  int getClassID(void);
  static std::list<supla_http_request *> createInTheCallerContext(
      supla_user *user, int DeviceId, int ChannelId, event_type EventType,
      const supla_caller &caller);
};

#define REGISTER_HTTP_REQUEST_CLASS(requestclass)                           \
  class requestclass##Factory : public AbstractHttpRequestFactory {         \
   public:                                                                  \
    requestclass##Factory();                                                \
    supla_http_request *create(supla_user *user, int ClassID, int DeviceId, \
                               int ChannelId, event_type EventType,         \
                               const supla_caller &caller);                 \
  };                                                                        \
  requestclass##Factory::requestclass##Factory()                            \
      : AbstractHttpRequestFactory() {}                                     \
  supla_http_request *requestclass##Factory::create(                        \
      supla_user *user, int ClassID, int DeviceId, int ChannelId,           \
      event_type EventType, const supla_caller &caller) {                   \
    return new requestclass(user, ClassID, DeviceId, ChannelId, EventType,  \
                            caller);                                        \
  }                                                                         \
  static requestclass##Factory global_##requestclass##Factory;

#endif /* HTTP_HTTPREQUEST_H_ */
