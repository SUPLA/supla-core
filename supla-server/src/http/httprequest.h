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

#define CORRELATIONTOKEN_MAXSIZE 2048

#include <sys/time.h>
#include <cstddef>
#include <list>
#include <string>
#include "http/trivialhttps.h"

class supla_user;
class supla_http_request_queue;

class supla_http_request {
 private:
  supla_trivial_http *http;
  supla_trivial_https *https;
  supla_user *user;
  int ClassID;
  short EventSourceType;
  int DeviceId;
  int ChannelId;
  char *correlationToken;
  struct timeval startTime;
  int timeoutUs;

 protected:
  supla_trivial_http *getHttp();
  supla_trivial_https *getHttps();

 public:
  supla_http_request(supla_user *user, int ClassID, int DeviceId, int ChannelId,
                     short EventSourceType);
  int getClassID(void);
  supla_user *getUser(void);
  void setEventSourceType(short EventSourceType);
  short getEventSourceType(void);
  void setDeviceId(int DeviceId);
  int getDeviceId(void);
  virtual void setChannelId(int ChannelId);
  int getChannelId(void);
  void setCorrelationToken(const char correlationToken[]);
  const char *getCorrelationTokenPtr(void);
  void setDelay(int delayUs);
  void setTimeout(int timeoutUs);
  int getTimeout(void);
  int timeLeft(struct timeval *now);
  bool isWaiting(struct timeval *now);
  bool timeout(struct timeval *now);

  virtual bool verifyExisting(supla_http_request *existing) = 0;
  virtual bool queueUp(void) = 0;
  virtual bool isEventSourceTypeAccepted(short eventSourceType,
                                         bool verification) = 0;
  virtual void execute(void *sthread) = 0;
  virtual void terminate(void *sthread);
  virtual ~supla_http_request();
};

class AbstractHttpRequestFactory {
 private:
  int ClassID;
  static std::list<AbstractHttpRequestFactory *> factories;
  virtual supla_http_request *create(supla_user *user, int ClassID,
                                     int DeviceId, int ChannelId,
                                     short EventSourceType) = 0;

 public:
  AbstractHttpRequestFactory(void);
  virtual ~AbstractHttpRequestFactory(void);
  int getClassID(void);
  static std::list<supla_http_request *> createByChannelEventSourceType(
      supla_user *user, int DeviceId, int ChannelId, short EventSourceType);
};

#define REGISTER_HTTP_REQUEST_CLASS(requestclass)                           \
  class requestclass##Factory : public AbstractHttpRequestFactory {         \
   public:                                                                  \
    requestclass##Factory();                                                \
    supla_http_request *create(supla_user *user, int ClassID, int DeviceId, \
                               int ChannelId, short EventSourceType);       \
  };                                                                        \
  requestclass##Factory::requestclass##Factory()                            \
      : AbstractHttpRequestFactory() {}                                     \
  supla_http_request *requestclass##Factory::create(                        \
      supla_user *user, int ClassID, int DeviceId, int ChannelId,           \
      short EventSourceType) {                                              \
    return new requestclass(user, ClassID, DeviceId, ChannelId,             \
                            EventSourceType);                               \
  }                                                                         \
  static requestclass##Factory global_##requestclass##Factory;

#endif /* HTTP_HTTPREQUEST_H_ */
